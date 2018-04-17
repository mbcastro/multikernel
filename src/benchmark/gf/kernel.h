#ifndef _KERNEL_H_
#define _KERNEL_H_

/* Global definitions. */

#define MICRO (1.0/1000000)

// #define CHUNK_SIZE (512)     /* Maximum chunk size. */
#define CHUNK_SIZE (16)     /* Maximum chunk size. */

#define PI 3.14159265359    /* pi */
#define E 2.71828182845904  /* e */
#define SD 0.8              /* Standard deviation. */

/* Type of messages. */
#define MSG_CHUNK 1
#define MSG_DIE   0

/* RMEM definitions. */
#define SIZE_NCLUSTERS   (sizeof(int))
#define SIZE_MASKSIZE    (sizeof(int))
#define SIZE_IMGSIZE     (sizeof(int))

#define SIZE_MASK        (masksize * masksize * sizeof(double))
#define SIZE_IMAGE       (imgsize * imgsize * sizeof(unsigned char))
#define SIZE_NEWIMAGE    SIZE_IMAGE

#define OFF_NCLUSTERS  (0)
#define OFF_MASKSIZE   (OFF_NCLUSTERS + SIZE_NCLUSTERS)
#define OFF_IMGSIZE    (OFF_MASKSIZE  + SIZE_MASKSIZE)

#define OFF_MASK       (OFF_IMGSIZE   + SIZE_IMGSIZE) 
#define OFF_IMAGE      (OFF_MASK      + SIZE_MASK)
#define OFF_NEWIMAGE   (OFF_IMAGE     + SIZE_IMAGE)

#endif /* _KERNEL_H_ */