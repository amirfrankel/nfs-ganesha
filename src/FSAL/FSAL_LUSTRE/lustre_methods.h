/* VFS methods for handles
 */

/* private helpers from export
 */
char * lustre_get_root_path(struct fsal_export *exp_hdl) ;

/* method proto linkage to handle.c for export
 */

fsal_status_t lustre_lookup_path(struct fsal_export *exp_hdl,
				 const struct req_op_context *opctx,
				 const char *path,
				 struct fsal_obj_handle **handle);

fsal_status_t lustre_create_handle(struct fsal_export *exp_hdl,
				   const struct req_op_context *opctx,
				   struct gsh_buffdesc *hdl_desc,
				   struct fsal_obj_handle **handle);

/*
 * VFS internal object handle
 * handle is a pointer because
 *  a) the last element of file_handle is a char[] meaning variable len...
 *  b) we cannot depend on it *always* being last or being the only
 *     variable sized struct here...  a pointer is safer.
 * wrt locks, should this be a lock counter??
 * AF_UNIX sockets are strange ducks.  I personally cannot see why they
 * are here except for the ability of a client to see such an animal with
 * an 'ls' or get rid of one with an 'rm'.  You can't open them in the
 * usual file way so open_by_handle_at leads to a deadend.  To work around
 * this, we save the args that were used to mknod or lookup the socket.
 */

struct lustre_fsal_obj_handle {
	struct fsal_obj_handle obj_handle;
	struct lustre_file_handle *handle;
	union {
		struct {
			int fd;
			fsal_openflags_t openflags;
		} file;
		struct {
			unsigned char *link_content;
			int link_size;
		} symlink;
		struct {
			struct lustre_file_handle *sock_dir;
			char *sock_name;
		} sock;
	} u;
};


	/* I/O management */
fsal_status_t lustre_open(struct fsal_obj_handle *obj_hdl,
			  const struct req_op_context *opctx,
			  fsal_openflags_t openflags);
fsal_openflags_t lustre_status(struct fsal_obj_handle *obj_hdl);
fsal_status_t lustre_read(struct fsal_obj_handle *obj_hdl,
		       const struct req_op_context *opctx,
		       uint64_t offset,
		       size_t buffer_size,
		       void *buffer,
		       size_t *read_amount,
		       bool *end_of_file);
fsal_status_t lustre_write(struct fsal_obj_handle *obj_hdl,
			const struct req_op_context *opctx,
                        uint64_t offset,
			size_t buffer_size,
			void *buffer,
			size_t * write_amount,
			bool *fsal_stable);
fsal_status_t lustre_commit(struct fsal_obj_handle *obj_hdl, /* sync */
			 off_t offset,
			 size_t len);
fsal_status_t lustre_lock_op(struct fsal_obj_handle *obj_hdl,
			  const struct req_op_context *opctx,
			  void * p_owner,
			  fsal_lock_op_t lock_op,
			  fsal_lock_param_t *request_lock,
			  fsal_lock_param_t *conflicting_lock);
fsal_status_t lustre_share_op(struct fsal_obj_handle *obj_hdl,
			   void *p_owner,         /* IN (opaque to FSAL) */
			   fsal_share_param_t  request_share);
fsal_status_t lustre_close(struct fsal_obj_handle *obj_hdl);
fsal_status_t lustre_lru_cleanup(struct fsal_obj_handle *obj_hdl,
			      lru_actions_t requests);

/* extended attributes management */
fsal_status_t lustre_list_ext_attrs(struct fsal_obj_handle *obj_hdl,
                                    const struct req_op_context *opctx,
				    unsigned int cookie,
				    fsal_xattrent_t * xattrs_tab,
				    unsigned int xattrs_tabsize,
				    unsigned int *p_nb_returned,
				    int *end_of_list);
fsal_status_t lustre_getextattr_id_by_name(struct fsal_obj_handle *obj_hdl,
                                        const struct req_op_context *opctx,
					const char *xattr_name,
					unsigned int *pxattr_id);
fsal_status_t lustre_getextattr_value_by_name(struct fsal_obj_handle *obj_hdl,
                                           const struct req_op_context *opctx,
					   const char *xattr_name,
					   caddr_t buffer_addr,
					   size_t buffer_size,
					   size_t * p_output_size);
fsal_status_t lustre_getextattr_value_by_id(struct fsal_obj_handle *obj_hdl,
                                         const struct req_op_context *opctx,
					 unsigned int xattr_id,
					 caddr_t buffer_addr,
					 size_t buffer_size,
					 size_t *p_output_size);
fsal_status_t lustre_setextattr_value(struct fsal_obj_handle *obj_hdl,
                                   const struct req_op_context *opctx,
				   const char *xattr_name,
				   caddr_t buffer_addr,
				   size_t buffer_size,
				   int create);
fsal_status_t lustre_setextattr_value_by_id(struct fsal_obj_handle *obj_hdl,
                                         const struct req_op_context *opctx,
					 unsigned int xattr_id,
					 caddr_t buffer_addr,
					 size_t buffer_size);
fsal_status_t lustre_getextattr_attrs(struct fsal_obj_handle *obj_hdl,
                                   const struct req_op_context *opctx,
				   unsigned int xattr_id,
				   struct attrlist *p_attrs);
fsal_status_t lustre_remove_extattr_by_id(struct fsal_obj_handle *obj_hdl,
                                       const struct req_op_context *opctx,
				       unsigned int xattr_id);
fsal_status_t lustre_remove_extattr_by_name(struct fsal_obj_handle *obj_hdl,
                                         const struct req_op_context *opctx,
					 const char *xattr_name);
