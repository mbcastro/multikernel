/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define __NEED_HAL_NOC_
#define __NEED_HAL_CORE_
#include <hal.h>
#include <klib.h>

#include "noc.h"

/**
 * @brief Name for virtual NoC.
 */
#define UNIX_NOC_NAME "noc-virtual"

/**
 * @brief Name for virtual NoC lock.
 */
#define UNIX_NOC_LOCK_NAME "noc-lock"

/**
 * @brief Maximum number of interfaces in a cluster.
 */
#define NR_CLUSTER_INTERFACES                   \
	((HAL_NR_NOC_IONODES > HAL_NR_NOC_CNODES) ? \
		HAL_NR_NOC_IONODES : HAL_NR_NOC_CNODES)

/**
 * @brief NoC interface.
 */
struct noc_interface
{
	int used;      /* Used interface?        */
	pthread_t tid; /* ID of attached thread. */

	/**
	 * @brief Buffer.
	 */
	struct
	{
		int head; /* First element. */
		int tail; /* Last element.  */
	} buffer;
};

/**
 * @brief NoC clusters.
 */
struct noc_cluster
{
	int used;                              /* Used cluster?           */
	pid_t pid;                             /* ID of attached process. */
	int interfaces[NR_CLUSTER_INTERFACES]; /* Interfaces.             */
};

/**
 * @brief NoC.
 */
struct
{
	int refs;                           /* References.    */
	sem_t *lock;                        /* Lock.          */
	struct noc_cluster *clusters;       /* Clusters.      */
	struct noc_interface *interfaces;   /* Interfaces.    */
	int configuration[HAL_NR_CLUSTERS]; /* Configuration. */
} noc = {
	0,    /* References. */
	NULL, /* Lock.       */
	NULL, /* Clusters.   */
	NULL, /* Interfaces, */

	/* Configuration. */
	{
	/* IO clusters. */
		4, 4,

		/* Compute clusters. */
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1
	}
};

/**
 * @brief IDs of NoC nodes.
 */
const int hal_noc_nodes[HAL_NR_NOC_NODES] = {
	CCLUSTER0,
	CCLUSTER1,
	CCLUSTER2,
	CCLUSTER3,
	CCLUSTER4,
	CCLUSTER5,
	CCLUSTER6,
	CCLUSTER7,
	CCLUSTER8,
	CCLUSTER9,
	CCLUSTER10,
	CCLUSTER11,
	CCLUSTER12,
	CCLUSTER13,
	CCLUSTER14,
	CCLUSTER15,
	IOCLUSTER0 + 0,
	IOCLUSTER0 + 1,
	IOCLUSTER0 + 2,
	IOCLUSTER0 + 3,
	IOCLUSTER1 + 0,
	IOCLUSTER1 + 1,
	IOCLUSTER1 + 2,
	IOCLUSTER1 + 3,
};

/*============================================================================*
 * unix_noc_lock_init()                                                       *
 *============================================================================*/

/**
 * @brief Initializes the lock of the virtual NOC.
 */
static void unix_noc_lock_init(void)
{
	KASSERT((noc.lock = 
		sem_open(UNIX_NOC_LOCK_NAME,
			O_RDWR | O_CREAT,
			S_IRUSR | S_IWUSR,
			1)
		) != NULL
	);
}

/*============================================================================*
 * unix_noc_lock()                                                            *
 *============================================================================*/

/**
 * @brief Locks the virtual NoC.
 */
static inline void unix_noc_lock(void)
{
	KASSERT(sem_wait(noc.lock) != -1);
}

/*============================================================================*
 * unix_noc_unlock()                                                          *
 *============================================================================*/

/**
 * @brief Unlocks the virtual NoC.
 */
static inline void unix_noc_unlock(void)
{
	KASSERT(sem_post(noc.lock) != -1);
}

/*============================================================================*
 * unix_noc_interface_alloc()                                                 *
 *============================================================================*/

/**
 * @brief Allocates a virtual NoC interface.
 *
 * @returns The ID of the newly allocated virtual NoC interface.
 */
static int unix_noc_interface_alloc(void)
{
	/* Search for a free interface. */
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
	{
		/* Found. */
		if (!noc.interfaces[i].used)
		{
			noc.interfaces[i].used = 1;
			return (i);
		}
	}

	kpanic("cannot allocate a noc interface");

	/* Never gets here. */
	return (-1);
}

/*============================================================================*
 * unix_noc_interface_free()                                                  *
 *============================================================================*/

/**
 * @brief Frees a virtual NoC interface.
 */
static void unix_noc_interface_free(int interfaceid)
{
	noc.interfaces[interfaceid].used = 0;
}

/*============================================================================*
 * unix_noc_cluster_alloc()                                                   *
 *============================================================================*/

/**
 * @brief Allocates a virtual NoC cluster.
 *
 * @returns The ID of the newly allocated virtual cluster.
 */
static int unix_noc_cluster_alloc(void)
{
	/* Search for a free cluster. */
	for (int i = 0; i < HAL_NR_CLUSTERS; i++)
	{
		/* Found. */
		if (!noc.clusters[i].used)
		{
			noc.clusters[i].used = 1;
			return (i);
		}
	}

	kpanic("cannot allocate a noc cluster");
	
	/* Never gets here. */
	return (-1);
}

/*============================================================================*
 * unix_noc_cluster_free()                                                    *
 *============================================================================*/

/**
 * @brief Frees a virtual NoC cluster.
 *
 * @param clusterid ID of the target virtual NoC cluster.
 */
static void unix_noc_cluster_free(int clusterid)
{
	noc.clusters[clusterid].used = 0;
}

/*============================================================================*
 * unix_noc_attach()                                                          *
 *============================================================================*/

/**
 * @brief Attaches calling process to virtual NoC device.
 */
static void unix_noc_attach(void)
{
	int shm;
	void *p;
	struct stat st;
	int initialize = 0;
	size_t clusters_sz = HAL_NR_CLUSTERS*sizeof(struct noc_cluster);
	size_t interfaces_sz = HAL_NR_NOC_NODES*sizeof(struct noc_interface);

	/* Attach virtual NoC. */
	if (noc.refs > 0)
		return;

	unix_noc_lock_init();

	kprintf("attaching virtual noc");

	/* Open virtual NoC. */
	KASSERT((shm =
		shm_open(UNIX_NOC_NAME,
			O_RDWR | O_CREAT,
			S_IRUSR | S_IWUSR)
		) != -1
	);

	unix_noc_lock();

		/* Allocate virtual NoC. */
		KASSERT(fstat(shm, &st) != -1);
		if (st.st_size == 0)
		{
			kprintf("initializing virtual noc");
			initialize = 1;
			KASSERT(ftruncate(shm, clusters_sz + interfaces_sz) != -1);
		}

		/* Attach virtual NoC. */
		KASSERT((p = 
			mmap(NULL,
				clusters_sz + interfaces_sz,
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				shm,
				0)
			) != NULL
		);
		noc.clusters = p;
		noc.interfaces = (void *)(((char *)p) + clusters_sz);

		/* Initialize NoC. */
		if (initialize)
		{
			/* Interfaces. */
			for (int i = 0; i < HAL_NR_NOC_NODES; i++)
				noc.interfaces[i].used = 0;

			/* Clusters. */
			for (int i = 0; i < HAL_NR_CLUSTERS; i++)
			{
				noc.clusters[i].used = 0;
				for (int j = 0; j < noc.configuration[i]; j++)
					noc.clusters[i].interfaces[j] = -1;
			}
		}

	unix_noc_unlock();
}

/*============================================================================*
 * unix_noc_detach()                                                          *
 *============================================================================*/

/**
 * @brief Detaches the calling process from the virtual NoC device.
 */
static void unix_noc_detach(void)
{
	size_t clusters_sz = HAL_NR_CLUSTERS*sizeof(struct noc_cluster);
	size_t interfaces_sz = HAL_NR_NOC_NODES*sizeof(struct noc_interface);

	/* Attach virtual NoC. */
	if (noc.refs > 0)
		return;

	KASSERT(munmap(noc.clusters, clusters_sz + interfaces_sz) != -1);
	if (shm_unlink(UNIX_NOC_NAME) == -1)
		kprintf("cannot destroy lock of virtual noc");
	if (sem_unlink(UNIX_NOC_LOCK_NAME) != -1)
		kprintf("cannot destroy virtual noc");
}

/*============================================================================*
 * unix_noc_cluster_attach()                                                  *
 *============================================================================*/

/**
 * @brief Attaches the calling process to a virtual NoC cluster.
 *
 * @returns The ID of the virtual NoC cluster where the calling
 * process was attached to.
 */
static int unix_noc_cluster_attach(void)
{
	int clusterid;

	unix_noc_attach();

	unix_noc_lock();

		/*
		 * Check if the calling process
		 * is already attached to a cluster.
		 * If so, we have nothing else to do.
		 */
		for (int i = 0; i < HAL_NR_CLUSTERS; i++)
		{
			/*Skip invalid clusters. */
			if (!noc.clusters[i].used)
				continue;

			/* Found. */
			if (noc.clusters[i].pid == getpid())
			{
				clusterid = i;
				goto found_cluster;
			}
		}

		/* Attach calling process to interface. */
		clusterid = unix_noc_cluster_alloc();
		noc.clusters[clusterid].pid = getpid();

found_cluster:

	unix_noc_unlock();

	kprintf("process %d attached to cluster %d", getpid(), clusterid);

	return (clusterid);
}

/*============================================================================*
 * unix_noc_cluster_detach()                                                  *
 *============================================================================*/

/**
 * @brief Detaches the calling process from a virtual NoC cluster.
 */
static void unix_noc_cluster_detach(void)
{
	int clusterid = -1;

	unix_noc_lock();

		/*
		 * Search for the cluster where the 
		 * calling process is attached to.
		 */
		for (int i = 0; i < HAL_NR_CLUSTERS; i++)
		{
			/*Skip invalid clusters. */
			if (!noc.clusters[i].used)
				continue;

			/* Found. */
			if (noc.clusters[i].pid == getpid())
			{
				clusterid = i;
				goto found_cluster;
			}
		}

		kpanic("unattached process");

	found_cluster:

		noc.clusters[clusterid].pid = 0;
		unix_noc_cluster_free(clusterid);

		kprintf("process %d detached from cluster %d", getpid(), clusterid);

	unix_noc_unlock();

	unix_noc_detach();
}

/*============================================================================*
 * unix_noc_interface_attach()                                                *
 *============================================================================*/

/**
 * @brief Attaches the calling thread to a virtual NoC interface.
 */
static void unix_noc_interface_attach(void)
{
	int clusterid;
	int interfaceid;

	clusterid = unix_noc_cluster_attach();

	unix_noc_lock();

		/*
		 * Get the cluster in which the
		 * calling thread is attached to. 
		 */
		for (int i = 0; i < HAL_NR_CLUSTERS; i++)
		{
			/* Skip invalid clusters. */
			if (!noc.clusters[i].used)
				continue;

			/* Found. */
			if (noc.clusters[i].pid == getpid())
			{
				clusterid = i;
				goto found_cluster;
			}
		}

		kpanic("unattached process");

found_cluster:

		/*
		 * Check if the calling thread is
		 * attached to any interface in this cluster.
		 * If so, we have nothing else to do.
		 */
		for (int i = 0; i < noc.configuration[clusterid]; i++)
		{
			interfaceid = noc.clusters[clusterid].interfaces[i];

			/* Skip invalid interfaces. */
			if (interfaceid < 0)
				continue;

			KASSERT(noc.interfaces[interfaceid].used);

			/* Found. */
			if (noc.interfaces[interfaceid].tid == pthread_self())
				goto done;
		}

		/* Attach calling thread to interface. */
		interfaceid = unix_noc_interface_alloc();
		noc.interfaces[interfaceid].tid = pthread_self();
		noc.refs++;

		/* Initialize interface. */
		noc.interfaces[interfaceid].buffer.head = 0;
		noc.interfaces[interfaceid].buffer.tail = 0;

		/* Attach interface to cluster. */
		for (int i = 0; i < noc.configuration[clusterid]; i++)
		{
			/* Attach interface here. */
			if (noc.clusters[clusterid].interfaces[i] == -1)
			{
				kprintf("thread %d attached to interface %d", noc.refs, i);
				noc.clusters[clusterid].interfaces[i] = interfaceid;
				goto done;
			}
		}

		kpanic("cannot attach noc interface");

done:

	unix_noc_unlock();

}

/*============================================================================*
 * unix_noc_interface_detach()                                                *
 *============================================================================*/

/**
 * @brief Detaches the calling thread from a virtual NoC interface.
 */
static void unix_noc_interface_detach(void)
{
	int clusterid = -1;
	int interfaceid = -1;

	unix_noc_lock();

		/*
		 * Search for the cluster in which the
		 * calling process is attached to.
		 */
		for (int i = 0; i < HAL_NR_CLUSTERS; i++)
		{
			/* Skip invalid clusters. */
			if (!noc.clusters[i].used)
				continue;

			/* Found. */
			if (noc.clusters[i].pid == getpid())
			{
				clusterid = i;
				goto found_cluster;
			}
		}

		kpanic("unattached process");

	found_cluster:

		/*
		 * Search for the interface in which the
		 * calling thread is attached to.
		 */
		for (int i = 0; i < noc.configuration[clusterid]; i++)
		{
			interfaceid = noc.clusters[clusterid].interfaces[i];

			/* Skip invalid interfaces. */
			if (interfaceid < 0)
				continue;

			KASSERT(noc.interfaces[interfaceid].used);

			/* Found. */
			if (noc.interfaces[interfaceid].tid == pthread_self())
			{
				kprintf("thread %d detached from noc interface %d", noc.refs, i);
				noc.clusters[clusterid].interfaces[i] = -1;
				goto found_interface;
			}
		}

		kpanic("unattached thread");

	found_interface:

		/* Detach thread from interface. */
		noc.refs--;
		noc.interfaces[interfaceid].tid = 0;
		unix_noc_interface_free(interfaceid);

		/* Detach process from cluster. */
		for (int i = 0; i < noc.configuration[clusterid]; i++)
		{
			if (noc.clusters[clusterid].interfaces[i] >= 0)
			{
				unix_noc_unlock();
				return;
			}
		}

	unix_noc_unlock();

	unix_noc_cluster_detach();
}

/*============================================================================*
 * hal_get_node_id()                                                          *
 *============================================================================*/

/**
 * @brief Gets the ID of the NoC node attached to the underlying core.
 *
 * @returns The ID of the NoC node attached to the underlying core is
 * returned.
 *
 * @note This function is blocking.
 * @note This function is thread-safe.
 */
int hal_get_node_id(void)
{
	int nodeid = 0;
	int clusterid = -1;
	int interfaceid = -1;

	unix_noc_lock();

		/*
		 * Search for the cluster in which the
		 * calling process is attached to.
		 */
		for (int i = 0; i < HAL_NR_CLUSTERS; nodeid += noc.configuration[i++])
		{
			/* Skip invalid clusters. */
			if (!noc.clusters[i].used)
				continue;

			/* Found. */
			if (noc.clusters[i].pid == getpid())
			{
				clusterid = i;
				goto found_cluster;
			}
		}

		kpanic("unattached process");

	found_cluster:

		/*
		 * Search for the interface in which the
		 * calling thread is attached to.
		 */
		for (int i = 0; i < noc.configuration[clusterid]; i++, nodeid++)
		{
			interfaceid = noc.clusters[clusterid].interfaces[i];

			/* Skip invalid interfaces. */
			if (interfaceid < 0)
				continue;

			KASSERT(noc.interfaces[interfaceid].used);

			/* Found. */
			if (noc.interfaces[interfaceid].tid == pthread_self())
				goto found_interface;
		}

		kpanic("unattached thread");

	found_interface:

	unix_noc_unlock();

	return (nodeid);
}

/*============================================================================*
 * hal_get_node_num()                                                         *
 *============================================================================*/

/**
 * @brief Gets the logic number of a NoC node.
 *
 * @param nodeid ID of the target NoC node.
 *
 * @returns The logic number of the target NoC node.
 *
 * @note This function is non-blocking.
 * @note This function is thread-safe.
 */
int hal_get_node_num(int nodeid)
{
	/* Lookup table of NoC node IDs. */
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
	{
		/* Found. */
		if (nodeid == hal_noc_nodes[i])
			return (i);
	}

	kpanic("querying bad node id");

	/* Never gets here. */
	return (-EINVAL);
}

/*============================================================================*
 * unix_noc_setup()                                                           *
 *============================================================================*/

/**
 * @brief Initializes the virtual NoC device.
 */
void unix_noc_setup(void)
{
	unix_noc_interface_attach();
}

/*============================================================================*
 * unix_noc_cleanups()                                                        *
 *============================================================================*/

/**
 * @brief Shutdown the virtual NoC device.
 */
void unix_noc_cleanup(void)
{
	unix_noc_interface_detach();
}
