/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 */

/**
 *
 * \file    fsal_rcp.c
 * \date    $Date: 2006/01/24 13:45:37 $
 * \version $Revision: 1.7 $
 * \brief   Transfer operations.
 *
 */
#include "config.h"

#include <unistd.h>
#include "fsal.h"
#include "fsal_internal.h"
#include "fsal_convert.h"
#include "abstract_mem.h"

/**
 * FSAL_rcp:
 * Copy an HPSS file to/from a local filesystem.
 *
 * \param filehandle (input):
 *        Handle of the HPSS file to be copied.
 * \param p_context (input):
 *        Authentication context for the operation (user,...).
 * \param p_local_path (input):
 *        Path of the file in the local filesystem.
 * \param transfer_opt (input):
 *        Flags that indicate transfer direction and options.
 *        This consists of an inclusive OR between the following values :
 *        - FSAL_RCP_FS_TO_LOCAL: Copy the file from the filesystem
 *          to a local path.
 *        - FSAL_RCP_LOCAL_TO_FS: Copy the file from local path
 *          to the filesystem.
 *        - FSAL_RCP_LOCAL_CREAT: Create the target local file
 *          if it doesn't exist.
 *        - FSAL_RCP_LOCAL_EXCL: Produce an error if the target local file
 *          already exists.
 *
 * \return Major error codes :
 *      - ERR_FSAL_NO_ERROR     (no error)
 *      - ERR_FSAL_ACCESS       (user doesn't have the permissions for opening the file)
 *      - ERR_FSAL_STALE        (filehandle does not address an existing object) 
 *      - ERR_FSAL_INVAL        (filehandle does not address a regular file,
 *                               or tranfert options are conflicting)
 *      - ERR_FSAL_FAULT        (a NULL pointer was passed as mandatory argument)
 *      - Other error codes can be returned :
 *        ERR_FSAL_IO, ERR_FSAL_NOSPC, ERR_FSAL_DQUOT...
 */

fsal_status_t HPSSFSAL_rcp(hpssfsal_handle_t * filehandle,      /* IN */
                           hpssfsal_op_context_t * p_context,   /* IN */
                           fsal_path_t * p_local_path,  /* IN */
                           fsal_rcpflag_t transfer_opt  /* IN */
    )
{

  int local_fd;
  int local_flags;

  hpssfsal_file_t fs_fd;
  fsal_openflags_t fs_flags;

  fsal_status_t st = FSAL_STATUS_NO_ERROR;

  /* default buffer size for RCP: 1MB */
#define RCP_BUFFER_SIZE 1048576
  caddr_t IObuffer;

  int to_local = false;
  int to_fs = false;

  int eof = false;

  ssize_t local_size = 0;
  fsal_size_t fs_size;

  /* sanity checks. */

  if(!filehandle || !p_context || !p_local_path)
    Return(ERR_FSAL_FAULT, 0, INDEX_FSAL_rcp);

  to_local = ((transfer_opt & FSAL_RCP_FS_TO_LOCAL) == FSAL_RCP_FS_TO_LOCAL);
  to_fs = ((transfer_opt & FSAL_RCP_LOCAL_TO_FS) == FSAL_RCP_LOCAL_TO_FS);

  if(to_local)
    LogFullDebug(COMPONENT_FSAL,
                      "FSAL_rcp: FSAL -> local file (%s)", p_local_path->path);
  if(to_fs)
    LogFullDebug(COMPONENT_FSAL,
                      "FSAL_rcp: local file -> FSAL (%s)", p_local_path->path);

  /* must give the sens of transfert (exactly one) */

  if((!to_local && !to_fs) || (to_local && to_fs))
    Return(ERR_FSAL_INVAL, 0, INDEX_FSAL_rcp);

  /* first, open local file with the correct flags */

  if(to_fs)
    {
      local_flags = O_RDONLY;
    }
  else
    {
      local_flags = O_WRONLY | O_TRUNC;

      if((transfer_opt & FSAL_RCP_LOCAL_CREAT) == FSAL_RCP_LOCAL_CREAT)
        local_flags |= O_CREAT;

      if((transfer_opt & FSAL_RCP_LOCAL_EXCL) == FSAL_RCP_LOCAL_EXCL)
        local_flags |= O_EXCL;

    }

  if(isFullDebug(COMPONENT_FSAL))
  {

    char msg[1024];

    msg[0] = '\0';

    if((local_flags & O_RDONLY) == O_RDONLY)
      strcat(msg, "O_RDONLY ");

    if((local_flags & O_WRONLY) == O_WRONLY)
      strcat(msg, "O_WRONLY ");

    if((local_flags & O_TRUNC) == O_TRUNC)
      strcat(msg, "O_TRUNC ");

    if((local_flags & O_CREAT) == O_CREAT)
      strcat(msg, "O_CREAT ");

    if((local_flags & O_EXCL) == O_EXCL)
      strcat(msg, "O_EXCL ");

    LogFullDebug(COMPONENT_FSAL, "Openning local file %s with flags: %s",
                      p_local_path->path, msg);

  }

  local_fd = open(p_local_path->path, local_flags, 0644);

  if(local_fd == -1)
    {
      Return(hpss2fsal_error(errno), errno, INDEX_FSAL_rcp);
    }

  /* call FSAL_open with the correct flags */

  if(to_fs)
    {
      fs_flags = FSAL_O_WRONLY | FSAL_O_TRUNC;

      /* invalid flags for local to filesystem */

      if(((transfer_opt & FSAL_RCP_LOCAL_CREAT) == FSAL_RCP_LOCAL_CREAT)
         || ((transfer_opt & FSAL_RCP_LOCAL_EXCL) == FSAL_RCP_LOCAL_EXCL))
        {
          /* clean & return */
          close(local_fd);
          Return(ERR_FSAL_INVAL, 0, INDEX_FSAL_rcp);
        }
    }
  else
    {
      fs_flags = FSAL_O_RDONLY;
    }

  if(isFullDebug(COMPONENT_FSAL))
  {

    char msg[1024];

    msg[0] = '\0';

    if((fs_flags & FSAL_O_RDONLY) == FSAL_O_RDONLY)
      strcat(msg, "FSAL_O_RDONLY ");

    if((fs_flags & FSAL_O_WRONLY) == FSAL_O_WRONLY)
      strcat(msg, "FSAL_O_WRONLY ");

    if((fs_flags & FSAL_O_TRUNC) == FSAL_O_TRUNC)
      strcat(msg, "FSAL_O_TRUNC ");

    LogFullDebug(COMPONENT_FSAL, "Openning FSAL file with flags: %s", msg);

  }

  st = HPSSFSAL_open(filehandle, p_context, fs_flags, &fs_fd, NULL);

  if(FSAL_IS_ERROR(st))
    {
      /* clean & return */
      close(local_fd);
      Return(st.major, st.minor, INDEX_FSAL_rcp);
    }
  LogFullDebug(COMPONENT_FSAL,
                    "Allocating IO buffer of size %llu",
                    (unsigned long long)RCP_BUFFER_SIZE);

  /* Allocates buffer */

  IObuffer = gsh_malloc(RCP_BUFFER_SIZE);

  if(IObuffer == NULL)
    {
      /* clean & return */
      close(local_fd);
      HPSSFSAL_close(&fs_fd);
      Return(ERR_FSAL_NOMEM, ENOMEM, INDEX_FSAL_rcp);
    }

  /* read/write loop */

  while(!eof)
    {
      /* initialize error code */
      st = FSAL_STATUS_NO_ERROR;

      LogFullDebug(COMPONENT_FSAL, "Read a block from source");

      /* read */

      if(to_fs)                 /* from local filesystem */
        {
          local_size = read(local_fd, IObuffer, RCP_BUFFER_SIZE);

          if(local_size == -1)
            {
              st.major = ERR_FSAL_IO;
              st.minor = errno;
              break;            /* exit loop */
            }

          eof = (local_size == 0);

        }
      else                      /* from FSAL filesystem */
        {
          fs_size = 0;
          st = HPSSFSAL_read(&fs_fd, NULL, RCP_BUFFER_SIZE, IObuffer, &fs_size, &eof);

          if(FSAL_IS_ERROR(st))
            break;              /* exit loop */

        }

      /* write (if not eof) */

      if(!eof || ((!to_fs) && (fs_size > 0)))
        {
          LogFullDebug(COMPONENT_FSAL, "Write a block to destination");

          if(to_fs)             /* to FSAL filesystem */
            {

              st = HPSSFSAL_write(&fs_fd, p_context, NULL, local_size, IObuffer, &fs_size);

              if(FSAL_IS_ERROR(st))
                break;          /* exit loop */

            }
          else                  /* to local filesystem */
            {

              local_size = write(local_fd, IObuffer, fs_size);

              if(local_size == -1)
                {
                  st.major = ERR_FSAL_IO;
                  st.minor = errno;
                  break;        /* exit loop */
                }

            }                   /* if to_fs */

        }                       /* if eof */
      else
        LogFullDebug(COMPONENT_FSAL, "End of source file reached");

    }                           /* while !eof */

  /* Clean */

  gsh_free(IObuffer);
  close(local_fd);
  HPSSFSAL_close(&fs_fd);

  /* return status. */

  Return(st.major, st.minor, INDEX_FSAL_rcp);
}
