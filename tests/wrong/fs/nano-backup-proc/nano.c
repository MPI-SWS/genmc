/* Taken from GNU nano -- git commit 94f49e20 */

/**************************************************************************
 *   files.c  --  This file is part of GNU nano.                          *
 *                                                                        *
 *   Copyright (C) 1999-2011, 2013-2020 Free Software Foundation, Inc.    *
 *   Copyright (C) 2015-2020 Benno Schulenberg                            *
 *                                                                        *
 *   GNU nano is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published    *
 *   by the Free Software Foundation, either version 3 of the License,    *
 *   or (at your option) any later version.                               *
 *                                                                        *
 *   GNU nano is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty          *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.              *
 *   See the GNU General Public License for more details.                 *
 *                                                                        *
 *   You should have received a copy of the GNU General Public License    *
 *   along with this program.  If not, see http://www.gnu.org/licenses/.  *
 *                                                                        *
 **************************************************************************/

/* Read all data from inn, and write it to out.  File inn must be open for
 * reading, and out for writing.  Return 0 on success, a negative number on
 * read error, and a positive number on write error.  File inn is always
 * closed by this function, out is closed  only if close_out is true. */
int copy_file(FILE *inn, FILE *out, bool close_out)
{
	int retval = 0;
	char buf[BUFSIZ];
	size_t charsread;
	int (*flush_out_fnc)(FILE *) = (close_out) ? fclose : fflush;

	do {
		charsread = fread(buf, sizeof(char), BUFSIZ, inn);
		if (charsread == 0 && ferror(inn)) {
			retval = -1;
			break;
		}
		if (fwrite(buf, sizeof(char), charsread, out) < charsread) {
			retval = 2;
			break;
		}
	} while (charsread > 0);

	if (fclose(inn) == EOF)
		retval = -3;
	/* if (flush_out_fnc(out) == EOF) */
	/* 	retval = 4; */

	return retval;
}

/* Write the current buffer to disk.  If thefile isn't NULL, we write to a
 * temporary file that is already open.  If tmp is TRUE (when spell checking
 * or emergency dumping, for example), we set the umask to disallow anyone else
 * from accessing the file, and don't print out how many lines we wrote on the
 * status bar.  If method is APPEND or PREPEND, it means we will be appending
 * or prepending instead of overwriting the given file.  If fullbuffer is TRUE
 * and when writing normally, we set the current filename and stat info.
 * Return TRUE on success, and FALSE otherwise. */
bool write_file(const char *name, FILE *thefile, bool tmp,
		kind_of_writing_type method, bool fullbuffer)
{
#ifndef NANO_TINY
	bool is_existing_file;
		/* Becomes TRUE when the file is non-temporary and exists. */
	struct stat st;
		/* The status fields filled in by stat(). */
#define backupname __stringify(BACKUPNAME)
	/* char *backupname = NULL; */
		/* The name of the backup file, in case we make one. */
#endif
#define realname __stringify(FILENAME)
	/* char *realname = real_dir_from_tilde(name); */
		/* The filename after tilde expansion. */
#define tempname "tempname"
	/* char *tempname = NULL; */
		/* The name of the temporary file we use when prepending. */
	linestruct *line = openfile->filetop;
		/* An iterator for moving through the lines of the buffer. */
	size_t lineswritten = 0;
		/* The number of lines written, for feedback on the status bar. */
	bool retval = FALSE;
		/* The return value, to become TRUE when writing has succeeded. */

#ifdef ENABLE_OPERATINGDIR
	/* If we're writing a temporary file, we're probably going outside
	 * the operating directory, so skip the operating directory test. */
	if (!tmp && outside_of_confinement(realname, FALSE)) {
		statusline(ALERT, _("Can't write outside of %s"), operating_dir);
		goto cleanup_and_exit;
	}
#endif
#ifndef NANO_TINY
	/* Check whether the file (at the end of the symlink) exists. */
	is_existing_file = (!tmp) && (stat(realname, &st) != -1);

	/* If we haven't stat()d this file before (say, the user just specified
	 * it interactively), stat and save the value now, or else we will chase
	 * null pointers when we do modtime checks and such during backup. */
	if (openfile->statinfo == NULL && is_existing_file)
		stat_with_alloc(realname, &openfile->statinfo);

	/* When the user requested a backup, we do this only if the file exists and
	 * isn't temporary AND the file has not been modified by someone else since
	 * we opened it (or we are appending/prepending or writing a selection). */
	if (ISSET(MAKE_BACKUP) && is_existing_file && openfile->statinfo &&
						(openfile->statinfo->st_mtime == st.st_mtime ||
						method != OVERWRITE || openfile->mark)) {
		static struct timespec filetime[2];
		int backup_cflags, backup_fd, verdict;
		FILE *original = NULL, *backup_file = NULL;

		/* Remember the original file's access and modification times. */
		filetime[0].tv_sec = openfile->statinfo->st_atime;
		filetime[1].tv_sec = openfile->statinfo->st_mtime;

		/* Open the file of which a backup must be made. */
		original = fopen(realname, "rb");

		/* If we can't read from the original file, go on, since saving
		 * only the current buffer is better than saving nothing. */
		if (original == NULL) {
			statusline(ALERT, _("Error reading %s: %s"), realname, strerror(errno));
			goto skip_backup;
		}

		/* If no backup directory was specified, we make a simple backup
		 * by appending a tilde to the original file name.  Otherwise,
		 * we create a numbered backup in the specified directory. */
		if (backup_dir == NULL) {
			/* backupname = charalloc(strlen(realname) + 2); */
			sprintf(backupname, "%s~", realname);
		} else {
			char *backuptemp = get_full_path(realname);

			/* If we have a valid absolute path, replace each slash
			 * in this full path with an exclamation mark.  Otherwise,
			 * just use the file-name portion of the given path. */
			if (backuptemp) {
				for (int i = 0; backuptemp[i] != '\0'; i++)
					if (backuptemp[i] == '/')
						backuptemp[i] = '!';
			} else
				backuptemp = copy_of(tail(realname));

			/* backupname = charalloc(strlen(backup_dir) + strlen(backuptemp) + 1); */
			sprintf(backupname, "%s%s", backup_dir, backuptemp);
			free(backuptemp);

			backuptemp = get_next_filename(backupname, "~");
			free(backupname);
			/* backupname = backuptemp; */

			/* If all numbered backup names are taken, the user must
			 * be fond of backups.  Thus, without one, do not go on. */
			/* if (*backupname == '\0') { */
			/* 	statusline(ALERT, _("Too many existing backup files")); */
			/* 	goto cleanup_and_exit; */
			/* } */
		}

		/* Now first try to delete an existing backup file. */
		if (unlink(backupname) < 0 && errno != ENOENT && !ISSET(INSECURE_BACKUP)) {
			warn_and_briefly_pause(_("Cannot delete existing backup"));
			if (user_wants_to_proceed())
				goto skip_backup;
			statusline(HUSH, _("Cannot delete backup %s: %s"),
								backupname, strerror(errno));
			goto cleanup_and_exit;
		}

		if (ISSET(INSECURE_BACKUP))
			backup_cflags = O_WRONLY | O_CREAT | O_TRUNC;
		else
			backup_cflags = O_WRONLY | O_CREAT | O_EXCL;

		/* Create the backup file (or truncate the existing one). */
		backup_fd = open(backupname, backup_cflags, S_IRUSR|S_IWUSR);

		if (backup_fd >= 0)
			backup_file = fdopen(backup_fd, "wb");

		if (backup_file == NULL) {
			warn_and_briefly_pause(_("Cannot create backup file"));
			if (user_wants_to_proceed())
				goto skip_backup;
			statusline(HUSH, _("Cannot create backup %s: %s"),
								backupname, strerror(errno));
			goto cleanup_and_exit;
		}

		/* Try to change owner and group to those of the original file;
		 * ignore errors, as a normal user cannot change the owner. */
		IGNORE_CALL_RESULT(fchown(backup_fd, openfile->statinfo->st_uid,
											openfile->statinfo->st_gid));

		/* Set the backup's permissions to those of the original file.
		 * It is not a security issue if this fails, as we have created
		 * the file with just read and write permission for the owner. */
		IGNORE_CALL_RESULT(fchmod(backup_fd, openfile->statinfo->st_mode));

		/* Copy the existing file to the backup. */
		verdict = copy_file(original, backup_file, FALSE);

#ifdef MAKE_BACKUP_SAFE
		fsync(backup_fd);
#endif

		if (verdict < 0) {
			fclose(backup_file);
			statusline(ALERT, _("Error reading %s: %s"), realname, strerror(errno));
			goto cleanup_and_exit;
		} else if (verdict > 0) {
			fclose(backup_file);
			warn_and_briefly_pause(_("Cannot write backup file"));
			if (user_wants_to_proceed())
				goto skip_backup;
			statusline(HUSH, _("Cannot write backup %s: %s"),
								backupname, strerror(errno));
			goto cleanup_and_exit;
		}

		/* Set the backup's timestamps to those of the original file.
		 * Failure is unimportant: saving the file apparently worked. */
		IGNORE_CALL_RESULT(futimens(backup_fd, filetime));

		if (fclose(backup_file) != 0) {
			warn_and_briefly_pause(_("Cannot write backup"));
			if (user_wants_to_proceed())
				goto skip_backup;
			statusline(HUSH, _("Cannot write backup %s: %s"),
								backupname, strerror(errno));
			goto cleanup_and_exit;
		}
	}

  skip_backup:
	/* When prepending, first copy the existing file to a temporary file. */
	if (method == PREPEND) {
		FILE *source = fopen(realname, "rb");
		FILE *target = NULL;
		int verdict;

		if (source == NULL) {
			statusline(ALERT, _("Error reading %s: %s"), realname, strerror(errno));
			goto cleanup_and_exit;
		}

		/* tempname = safe_tempfile(&target); */

		/* if (tempname == NULL) { */
		/* 	statusline(ALERT, _("Error writing temp file: %s"), strerror(errno)); */
		/* 	goto cleanup_and_exit; */
		/* } */

		verdict = copy_file(source, target, TRUE);

		if (verdict < 0) {
			statusline(ALERT, _("Error reading %s: %s"), realname, strerror(errno));
			unlink(tempname);
			goto cleanup_and_exit;
		} else if (verdict > 0) {
			statusline(ALERT, _("Error writing temp file: %s"), strerror(errno));
			unlink(tempname);
			goto cleanup_and_exit;
		}
	}

	if (is_existing_file && S_ISFIFO(st.st_mode))
		statusbar(_("Writing to FIFO..."));
#endif /* !NANO_TINY */

	/* When it's not a temporary file, this is where we open or create it.
	 * For an emergency file, access is restricted to just the owner. */
	if (thefile == NULL) {
		mode_t permissions = (tmp ? S_IRUSR|S_IWUSR : RW_FOR_ALL);
		int fd;

#ifndef NANO_TINY
		block_sigwinch(TRUE);
		install_handler_for_Ctrl_C();
#endif

		/* Now open the file.  Use O_EXCL for an emergency file. */
		fd = open(realname, O_WRONLY | O_CREAT | ((method == APPEND) ?
					O_APPEND : (tmp ? O_EXCL : O_TRUNC)), permissions);

#ifndef NANO_TINY
		restore_handler_for_Ctrl_C();
		block_sigwinch(FALSE);
#endif

		/* If we couldn't open the file, give up. */
		if (fd == -1) {
			if (errno == EINTR || errno == 0)
				statusline(ALERT, _("Interrupted"));
			else
				statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
#ifndef NANO_TINY
			if (tempname != NULL)
				unlink(tempname);
#endif
			goto cleanup_and_exit;
		}

		thefile = fdopen(fd, (method == APPEND) ? "ab" : "wb");

		if (thefile == NULL) {
			statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
			close(fd);
			goto cleanup_and_exit;
		}
	}

	if (!tmp)
		statusbar(_("Writing..."));

	while (TRUE) {
#define data_len strlen(BUFFER_DATA)
		/* size_t data_len = strlen(line->data); */
		size_t wrote;

		/* Decode LFs as the NULs that they are, before writing to disk. */
		recode_LF_to_NUL(line->data);

		wrote = fwrite(line->data, sizeof(char), data_len, thefile);

		/* Re-encode any embedded NULs as LFs. */
		recode_NUL_to_LF(line->data, data_len);

		if (wrote < data_len) {
			statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
			fclose(thefile);
			goto cleanup_and_exit;
		}

		/* If we've reached the last line of the buffer, don't write a newline
		 * character after it.  If this last line is empty, it means zero bytes
		 * are written for it, and we don't count it in the number of lines. */
		if (line->next == NULL) {
			if (line->data[0] != '\0')
				lineswritten++;
			break;
		}

#ifndef NANO_TINY
		if (openfile->fmt == DOS_FILE || openfile->fmt == MAC_FILE) {
			if (putc('\r', thefile) == EOF) {
				statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
				fclose(thefile);
				goto cleanup_and_exit;
			}
		}

		if (openfile->fmt != MAC_FILE)
#endif
			if (putc('\n', thefile) == EOF) {
				statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
				fclose(thefile);
				goto cleanup_and_exit;
			}

		line = line->next;
		lineswritten++;
	}

#ifndef NANO_TINY
	/* When prepending, append the temporary file to what we wrote above. */
	if (method == PREPEND) {
		FILE *source = fopen(tempname, "rb");
		int verdict;

		if (source == NULL) {
			statusline(ALERT, _("Error reading temp file: %s"), strerror(errno));
			fclose(thefile);
			goto cleanup_and_exit;
		}

		verdict = copy_file(source, thefile, TRUE);

		if (verdict < 0) {
			statusline(ALERT, _("Error reading temp file: %s"), strerror(errno));
			goto cleanup_and_exit;
		} else if (verdict > 0) {
			statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
			goto cleanup_and_exit;
		}

		unlink(tempname);
	} else
#endif
	if (fclose(thefile) != 0) {
		statusline(ALERT, _("Error writing %s: %s"), realname, strerror(errno));
		goto cleanup_and_exit;
	}

	/* When having written an entire buffer, update some administrivia. */
	if (fullbuffer && method == OVERWRITE && !tmp) {
		/* If the filename was changed, write a new lockfile when needed,
		 * and check whether it means a different syntax gets used. */
		if (strcmp(openfile->filename, realname) != 0) {
#ifndef NANO_TINY
			if (openfile->lock_filename != NULL) {
				delete_lockfile(openfile->lock_filename);
				free(openfile->lock_filename);
				openfile->lock_filename = do_lockfile(realname, FALSE);
			}
#endif
			openfile->filename = mallocstrcpy(openfile->filename, realname);
#ifdef ENABLE_COLOR
			const char *oldname, *newname;

			oldname = openfile->syntax ? openfile->syntax->name : "";
			find_and_prime_applicable_syntax();
			newname = openfile->syntax ? openfile->syntax->name : "";

			/* If the syntax changed, discard and recompute the multidata. */
			if (strcmp(oldname, newname) != 0) {
				for (line = openfile->filetop; line != NULL; line = line->next) {
					free(line->multidata);
					line->multidata = NULL;
				}

				precalc_multicolorinfo();
				have_palette = FALSE;
				refresh_needed = TRUE;
			}
#endif
		}
#ifndef NANO_TINY
		/* Get or update the stat info to reflect the current state. */
		stat_with_alloc(realname, &openfile->statinfo);

		/* Record at which point in the undo stack the file was saved. */
		openfile->last_saved = openfile->current_undo;
		openfile->last_action = OTHER;
#endif
		openfile->modified = FALSE;
		titlebar(NULL);
	}

	if (!tmp)
		statusline(HUSH, P_("Wrote %zu line", "Wrote %zu lines",
								lineswritten), lineswritten);
	retval = TRUE;

  cleanup_and_exit:
#ifndef NANO_TINY
	free(backupname);
#endif
	free(tempname);
	free(realname);

	return retval;
}
