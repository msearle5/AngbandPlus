;;; -*- Mode: Lisp; Syntax: Common-Lisp; Package: cl-user -*-

#|

DESC: pre-build.lisp - settings that must be set before build
Copyright (c) 2001-2002 - Stig Erik Sand�

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

|#

(in-package :cl-user)

#+clisp
(progn
  (format t "~&Removing some clisp-warnings.. we hope~%")
  ;;(push (pathname "@lisppath@/") *load-paths*)        
  (setq 
   clos::*gf-warn-on-removing-all-methods* nil
   clos::*warn-if-gf-already-called* nil
   clos::*gf-warn-on-replacing-method* nil
   system::*source-file-types* '(".lisp" ".lsp")))

#+cormanlisp
(setq cl::*support-eql-specializers* t)
  
#+ecl
(setq sys:*gc-verbose* nil)

  
#+cmu
(progn
  (setq ext:*gc-verbose* nil
	ext:*byte-compile-default* nil
	*compile-print* nil)
  #+pcl
  (pushnew 'compile pcl::*defclass-times*))

#+sbcl
(progn
  (setq ;;sb-ext:*gc-verbose* nil
	;;sb-ext:*byte-compile-default* nil
	*compile-print* nil
	))
  

#+allegro
(progn
  (setf *load-local-names-info* t
	;;(sys:gsgc-switch :print) t
	))
 
(defun %load-settings-file (fname)
  "Loads settings info."
  (with-open-file (s (pathname fname)
		     :direction :input)
    (loop for x = (read s nil 'eof)
	  until (eq x 'eof)
	  do

	  (cond ((consp x)
		 (case (car x)
		   (sound-use
		    #-clisp
		    (when (eq (second x) 'yes)
		      (pushnew :using-sound *features*)))
		   (environments
		    (dolist (i (cdr x))
		      (when (eq i 'x11)
			(pushnew :image-support *features*))))
		   (otherwise
		    (warn "Unknown setting directive ~s in ~s" x fname))))
		(t
		 (warn "Unknwon setting directive ~s in ~s" x fname)))
	  )))

(ignore-errors
  (%load-settings-file "config/settings.cfg")) ;; fix
