/**
 * @brief name request types
 */
#define NAME_QUERY 0
#define NAME_ADD 1
#define NAME_REMOVE 2

/**
 * @brief name query message.
 */
struct name_message
{
  uint16_t source;     	/**< Source cluster.	*/
  uint16_t op;      		/**< Operation.     	*/
	int id;     					/**< Cluster ID.  		*/
	int dma;    					/**< DMA channel. 		*/
	const char *name;        	  /**< Portal name. 		*/
	const char *process_name;		/**< Process name. 		*/
};
