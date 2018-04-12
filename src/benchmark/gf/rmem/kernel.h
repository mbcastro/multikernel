#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <nanvix/arch/mppa.h>

#define MAX_MASK  (15)
#define MAX_IMAGE (32768)

#define LENGTH_MASK  (MAX_MASK  *  MAX_MASK)
#define LENGTH_IMAGE (MAX_IMAGE * MAX_IMAGE)

#define SIZE_NCLUSTERS   (sizeof(int))
#define SIZE_MASK_SIZE   (sizeof(int))
#define SIZE_IMAGE_SIZE  (sizeof(int))
#define SIZE_MASK        (LENGTH_MASK  * sizeof(double))
#define SIZE_IMAGE       (LENGTH_IMAGE * sizeof(unsigned char))

#define OFF_NCLUSTERS  (0)
#define OFF_MASK_SIZE  (OFF_NCLUSTERS  + SIZE_NCLUSTERS)   
#define OFF_IMAGE_SIZE (OFF_MASK_SIZE  + SIZE_MASK_SIZE)
#define OFF_MASK       (OFF_IMAGE_SIZE + SIZE_IMAGE_SIZE) 
#define OFF_IMAGE      (OFF_MASK       + SIZE_MASK)

#endif /* _KERNEL_H_ */
