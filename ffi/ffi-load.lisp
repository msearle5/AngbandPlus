;;; -*- Mode: Lisp; Syntax: Common-Lisp; Package: cl-user -*-

#|

DESC: ffi/ffi-load.lisp - settings that must be set before foreign build
Copyright (c) 2001 - Stig Erik Sand�

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

|#

(in-package :cl-user)

(eval-when (:execute :load-toplevel :compile-toplevel)

  (defvar *loaded-libs* '())
  
  (defun quickly-quit-game& ()
    "Tries to quit game.."
    #+cmu
    (cl-user::quit)
    #+allegro
    (excl::exit)
    #+sbcl
    (sb-ext:quit)
    #+openmcl
    (cl-user::quit)
    #-(or cmu allegro sbcl openmcl)
    (warn "Can't quit yet.. fix me..")
  (values))
  
  
  (defun load-shared-lib (&key
			  (lib "./zterm/lbui.so")
			  (key :unknown))
    "Loads the necessary shared-lib."
    #-lispworks
    (declare (ignore key))
    
    (let ((is-there (probe-file lib)))
      (unless is-there
	(warn "Unable to locate dynamic library ~a, please run 'make'."
	      lib)
	(quickly-quit-game&)))
    
    
    #+allegro
    (load lib)
    #+cmu
    (alien:load-foreign lib)
    #+clisp
    nil
    #+lispworks
    (fli:register-module key :real-name lib :connection-style :manual)
    #+sbcl
    (sb-alien:load-foreign lib)
    #+openmcl
    (ccl:open-shared-library lib)
    #-(or cmu allegro clisp lispworks sbcl openmcl)
    (warn "Did not load shared-library.."))
  
  )

(eval-when (:execute :load-toplevel :compile-toplevel)
  (let ((lib-path "./zterm/"))

;;   #+cmu
;;   (SYSTEM:FOREIGN-SYMBOL-ADDRESS "funcall0")

    #+unix
    (progn
      (setq lib-path
	    #+langband-development "./zterm/"
	    #-langband-development "/usr/lib/langband/"))
    #+win32
    (progn
      ;; hack
      (setq lib-path (concatenate 'string (lb-engine:lbsys/ensure-dir-name
					   (namestring (lb-engine:lbsys/get-current-directory)))
				  "zterm/")))

    #+unix
    (progn
      #-(or cmu sbcl)
      (unless (find :dc *loaded-libs*)
	(load-shared-lib :key :dc :lib (concatenate 'string lib-path "dircall.so"))
	(push :dc *loaded-libs*))
      
      ;; everyone
      (unless (find :ui *loaded-libs*)
	(load-shared-lib :key :lang-ffi :lib (concatenate 'string lib-path "lbui.so"))
	(push :ui *loaded-libs*)))
    
    #+win32
    (progn
      (load-shared-lib :key :lang-ffi :lib (concatenate 'string lib-path "lbui.dll"))
      
    )))

