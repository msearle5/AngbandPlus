;;; -*- Mode: Lisp; Syntax: Common-Lisp; Package: LANGBAND -*-

#|

DESC: save.lisp - saving of various parts of the game
Copyright (c) 2000-2001 - Stig Erik Sand�

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

|#

(in-package :langband)

(eval-when (:compile-toplevel :load-toplevel :execute)
  
  (defclass l-readable-stream ()
    ((the-stream :accessor lang.stream :initform nil :initarg :stream)))
  (defclass l-binary-stream ()
    ((the-stream :accessor lang.stream :initform nil :initarg :stream)))


  (defgeneric save-object (object stream indent)
    (:documentation "Tries to save object to the stream."))

  (defgeneric load-object (type stream)
    (:documentation "Tries to load a certain type of object from the stream."))
  
  )

(defun %bin-save-string (obj stream)
  "Tries to write a var-length string."
  (let ((len (length obj)))
    (bt:write-binary 'bt:u32 stream len)
    (dotimes (i len)
      (funcall bt:*binary-write-byte* (char-code (aref obj i)) stream))
    len))
 
(defun %bin-read-string (stream)
  "Tries to read a string from the given stream."
  (let* ((len (bt:read-binary 'bt:u32 stream))
	 (str (bt:read-fixed-size-string len stream)))
    (values str len)))


(defun %bin-read-array (len type str)
  "Tries to read an array from the stream STR."
  (let ((arr (make-array len)))
    (dotimes (i len)
      (setf (aref arr i) (bt:read-binary type str)))
    arr))

(defun %bin-write-array (arr type str)
  "Tries to write the array ARR to the stream STR."
  (dotimes (i (length arr))
;;    (warn "writing ~s as ~s" (aref arr i) type)
    (bt:write-binary type str (aref arr i))))

(defvar +long-space-word+ "                                ")
(defvar *save-hanger* nil)

(defun %get-indent-str (indent)
  (subseq +long-space-word+ 0 indent))

(defmethod save-object ((obj variant) (stream l-readable-stream) indent)

  (let ((str (lang.stream stream))
	(ind (%get-indent-str indent)))

    ;; do turn-events later
    (format str "~a(%filed-variant :id ~s :turn ~s)~%"
	    ind (string (variant.id obj)) (variant.turn obj))

    nil))

(defmethod save-object ((obj variant) (stream l-binary-stream) indent)
  (declare (ignore indent))
  (let ((str (lang.stream stream)))

    (%bin-save-string (string (variant.id obj)) str)
    (write-binary 'u32 str (variant.turn obj))
    
    nil))

 

(defmethod save-object ((object dungeon) (stream l-readable-stream) indent)
  ;;(print object stream)

  (let* ((width (dungeon.width object))
	 (height (dungeon.height object))
;;	 (old-table (dungeon.table object))
	 (str (lang.stream stream))
	 (ind (%get-indent-str indent))
	 (new-arr (make-array (list width height))))
    
    (with-dungeon (object (coord x y))
      (setf (aref new-arr x y) (cons (coord.feature coord)
				     (logand (coord.flags coord) +saved-cave-flags+)
				     )))
    

    (format str "~a(%filed-dungeon :height ~a :width ~a :depth ~a ~%"
	    ind height width (dungeon.depth object))
    
    (format str "~a :table ~s ~%" ind new-arr)
    
    (let ((monsters (dungeon.monsters object)))
      (when monsters
	(format str "~a :monsters (list ~%" ind)
	(dolist (i monsters)
	  (save-object i stream (+ 2 indent)))
	(format str "~a )~%" ind)))

    (let ((objs (dungeon.objects object)))
      (when objs
	(format str "~a :objects (list ~%" ind)
	(dolist (i objs)
	  (save-object i stream (+ 2 indent)))
	(format str "~a )~%" ind)))

    (let ((rooms (dungeon.rooms object)))
      (when rooms
	(format str "~a :rooms (list ~%" ind)
	(dolist (i rooms)
	  (save-object i stream (+ 2 indent)))
	(format str "~a )~%" ind)))
    
    (format str "~a) ;; end dng~%" ind)
    ))


(defmethod save-object ((object dungeon) (stream l-binary-stream) indent)
  (let ((str (lang.stream stream)))

    (write-binary 's16 str (dungeon.depth  object))
    (write-binary 'u16 str (dungeon.width  object))
    (write-binary 'u16 str (dungeon.height object))

    (with-dungeon (object (coord x y))
      (declare (ignore x y))
      (write-binary 'u16 str (coord.feature coord))
      (write-binary 'u16 str (logand (coord.flags coord)
				     +saved-cave-flags+)))

    ;; write monsters
    (let* ((monsters (dungeon.monsters object))
	   (mon-len (length monsters)))

      (bt:write-binary 'bt:u32 str mon-len)
      (dolist (i monsters)
	(save-object i stream indent)))
    
    ;; write objects
    (let* ((objects (dungeon.objects object))
	   (obj-len (length objects)))
      
      (bt:write-binary 'bt:u32 str obj-len)
      (dolist (i objects)
	(save-object i stream indent)))

    ;; write rooms
    (let* ((rooms (dungeon.rooms object))
	   (room-len (length rooms)))
      
      (bt:write-binary 'bt:u32 str room-len)
      (dolist (i rooms)
	(save-object i stream indent)))

    
    ))

(defmethod save-object ((object active-monster) (stream l-readable-stream) indent)
  (let ((str (lang.stream stream))
	(ind (%get-indent-str indent)))
    
    (format str "~a(%filed-monster :kind ~s :cur-hp ~a :max-hp ~a :speed ~a ~%"
	    ind (monster.id (amon.kind object))
	    (current-hp object)
	    (get-creature-max-hp object)
	    (get-creature-speed object))
    
    (format str "~a :energy ~a :mana ~a :loc-x ~a :loc-y ~a :alive? ~a) ;; end mon~%"
	    ind (get-creature-energy object)
	    (get-creature-mana object)
	    (location-x object)
	    (location-y object)
	    (creature-alive? object))
    nil))

  
(defmethod save-object ((object active-monster) (stream l-binary-stream) indent)
  (declare (ignore indent))
  
  (let ((str (lang.stream stream))
	(the-kind-id (monster.id (amon.kind object))))

    (assert (stringp the-kind-id))
    (%bin-save-string the-kind-id str)
    (bt:write-binary 's16 str (current-hp object))
    (bt:write-binary 'u16 str (get-creature-max-hp object))
    (bt:write-binary 'u16 str (get-creature-speed object))
    (bt:write-binary 'u16 str (get-creature-energy object))
    (bt:write-binary 'u16 str (get-creature-mana object))
    (bt:write-binary 'u16 str (location-x object))
    (bt:write-binary 'u16 str (location-y object))
    (bt:write-binary 'u16 str (if (creature-alive? object) 1 0))
			 
    nil))

(defmethod save-object ((object active-object) (stream l-readable-stream) indent)

  (assert (ok-object? object))
;;  (warn "saving r-object ~a" object)
  (let ((str (lang.stream stream))
	(ind (%get-indent-str indent))
	(the-kind-id (object.id (aobj.kind object)))
	(inscription (aobj.inscr object))
	(number (aobj.number object))
	(contains (aobj.contains object))
	(events (aobj.events object))
	(loc-x (location-x object))
	(loc-y (location-y object)))
    
    (format str "~a(%filed-object :kind ~s :inscr ~s :number ~s~%"
	    ind the-kind-id inscription number)
    
    (format str "~a:contains~%" ind)
    (if contains
	(save-object contains stream (+ 2 indent))
	(format str "~a  nil~%" ind))
    
    (format str "~a:events ~s :loc-x ~s :loc-y ~s) ;; end obj~%"
	    ind events loc-x loc-y)
    
    ))

(defmethod save-object ((object active-object) (stream l-binary-stream) indent)

  (assert (ok-object? object))
	
;;  (warn "saving b-object ~a" object)
  (let ((str (lang.stream stream))
	(the-kind-id (object.id (aobj.kind object)))
	(inscription (aobj.inscr object))
	(number (aobj.number object))
	(contains (aobj.contains object))
;;	(events (aobj.events object))
	(loc-x (location-x object))
	(loc-y (location-y object)))
    
    (%bin-save-string the-kind-id str)

    (%bin-save-string inscription str)

    (bt:write-binary 'u16 str number)
    (bt:write-binary 'u16 str loc-x)
    (bt:write-binary 'u16 str loc-y)
    
    ;; dump out a digit if we have containment
    (bt:write-binary 'u16 str (if contains 1 0))
    (when contains
      (save-object contains stream indent))
    
    ;; skip events


    nil))

(defmethod save-object ((obj active-room) (stream l-readable-stream) indent)
  
  (assert (ok-object? obj))

  (let ((str (lang.stream stream))
	(ind (%get-indent-str indent)))

    (format str "~a(%filed-room :type ~s :loc-x ~s :loc-y ~s)~%"
	    ind (room-type.id (room.type obj))
	    (location-x obj) (location-y obj))
    
    nil))

(defmethod save-object ((obj active-room) (stream l-binary-stream) indent)
  (declare (ignore indent))
  (assert (ok-object? obj))

  (let ((str (lang.stream stream))
	(loc-x (location-x obj))
	(loc-y (location-y obj))
	)

    (%bin-save-string (room-type.id (room.type obj)) str)
    (bt:write-binary 'u16 str loc-x)
    (bt:write-binary 'u16 str loc-y)
     
    nil))

(defmethod save-object ((obj player) (stream l-readable-stream) indent)
  (assert (ok-object? obj))
  (let ((str (lang.stream stream))
	(ind (%get-indent-str indent)))
	
    (format str "~a(%filed-player ~%" ind)
    (format str "~a  :name ~s :race ~s :class ~s :sex ~s ~%"
	    ind (player.name obj)
	    (string (race.id (player.race obj)))
	    (string (class.id (player.class obj)))
	    (string (player.sex obj))
	    )
    
    (format str "~a  :base-stats ~s :curbase-stats ~s ~%"
	    ind (player.base-stats obj) (player.curbase-stats obj))
    
    (format str "~a  :hp-table ~s ~%"
	    ind (player.hp-table obj))
    
    (format str "~a  :loc-x ~s :loc-y ~s :view-x ~s :view-y ~s ~%"
	    ind (location-x obj) (location-y obj)
	    (player.view-x obj) (player.view-y obj))
    
    (format str "~a  :depth ~s :max-depth ~s :max-xp ~s :cur-xp ~s :fraction-xp ~s ~%"
	    ind (player.depth obj) (player.max-depth obj)
	    (player.max-xp obj) (player.cur-xp obj)
	    (player.fraction-xp obj))
    
    (format str "~a  :cur-hp ~s :fraction-hp ~s :cur-mana ~s :fraction-mana ~s ~%"
	    ind (player.cur-hp obj) (player.fraction-hp obj)
	    (player.cur-mana obj) (player.fraction-mana obj))

    (format str "~a  :gold ~s :energy ~s ~%"
	    ind (player.gold obj) (player.energy obj))
    
    
    (format str "~a  :equipment ~%" ind)
    (save-object (player.equipment obj) stream (+ 2 indent))
  
    (format str " ~a) ;; end player ~%" ind)

    nil))

(defmethod save-object ((obj player) (stream l-binary-stream) indent)
  (let ((str (lang.stream stream)))
	
    (bt:write-binary 'player str obj)
    ;; then do the four first ones
    (%bin-save-string (player.name obj) str)
    (%bin-save-string (string (race.id (player.race obj))) str)
    (%bin-save-string (string (class.id (player.class obj))) str)
    (%bin-save-string (string (player.sex obj)) str)

    (%bin-write-array (player.base-stats obj) 'bt:u16 str)
    (%bin-write-array (player.curbase-stats obj) 'bt:u16 str)
    (%bin-write-array (player.hp-table obj) 'bt:u16 str)

    (save-object (player.equipment obj) stream indent)
    
    nil))


(defmethod save-object ((obj items-worn) (stream l-binary-stream) indent)
  (let ((str (lang.stream stream)))

    (bt:write-binary 'u16 str (items.cur-size obj))

    (item-table-iterate! obj #'(lambda (tbl num loc-obj)
				 (declare (ignore tbl num))
				 (if (not loc-obj)
				     (bt:write-binary 'u16 str 0)
				     (progn
				       (bt:write-binary 'u16 str 1)
				       (save-object loc-obj stream indent)))))
    nil))


(defmethod save-object ((obj items-worn) (stream l-readable-stream) indent)
  
  (let ((str (lang.stream stream))
	(ind (%get-indent-str indent)))
    
    (format str "~a(%filed-worn-items :objs (list " ind)

;;    (format str "nil")
    (flet ((save-objs (tbl num loc-obj)
	     (declare (ignore tbl num))
	     (if (not loc-obj)
		 (format str " nil ")
		 (save-object loc-obj stream (+ 2 indent)))))
      
      (item-table-iterate! obj #'save-objs))

    (format str "~a )) ;; end worn ~%" ind)
    
    nil))


(defmethod save-object ((obj items-in-container) (stream l-readable-stream) indent)
  (let ((str (lang.stream stream))
	(ind (%get-indent-str indent)))
    
    (format str "~a(%filed-contained-items :cur-size ~s :max-size ~s :objs (list "
	    ind (items.cur-size obj) (items.max-size obj))
    
    (flet ((save-objs (tbl num loc-obj)
	     (declare (ignore tbl num))
	     (if (not loc-obj)
		 (format str " nil ")
		 (save-object loc-obj stream (+ 2 indent)))))
      
      (item-table-iterate! obj #'save-objs))
    
    (format str "~a )) ;; end cont ~%" ind)
    
    nil))


(defmethod save-object ((obj items-in-container) (stream l-binary-stream) indent)
  (let ((str (lang.stream stream))
	(cur-size (items.cur-size obj))
	(max-size (items.max-size obj)))
    
    (bt:write-binary 'u16 str cur-size)
    (bt:write-binary 'u16 str max-size)

    (flet ((save-objs (tbl num loc-obj)
	     (declare (ignore tbl num))
	     (if (not loc-obj)
		 (error "NIL in the middle of a container")
		 (save-object loc-obj stream indent))))
      
      (item-table-iterate! obj #'save-objs))
				     
    nil))

(defmethod save-object ((obj level) (stream l-readable-stream) indent)
  (let ((str (lang.stream stream))
	(ind (%get-indent-str indent))
	(the-id (string (level.id obj))))

    ;; hackish
    (format str "~a(let* ((builder (get-level-builder ~s))~%"
	    ind the-id)
    (format str "~a       (*level* (funcall builder))) ;; evil hack~%"
	    ind)

    (format str "  ~a(%filed-level :id ~s :rating ~s :depth ~s ~%"
	    ind  the-id (level.rating obj) (level.depth obj))

    (format str "  ~a :dungeon ~%" ind)
    
    (save-object (level.dungeon obj) stream (+ 4 indent))
    
    (format str "  ~a)) ;; end lvl~%" ind)
    
    nil))

(defmethod save-object ((obj level) (stream l-binary-stream) indent)
  (let ((str (lang.stream stream)))

    (%bin-save-string (string (level.id obj)) str)
    (bt:write-binary 'u16 str (level.rating obj))
    (bt:write-binary 'u16 str (level.depth obj))
    (save-object (level.dungeon obj) stream indent)
    
    nil))
  

;;; === Move the ones below somewhere else later ===


(defmethod do-save ((type (eql :readable)) fname obj-list)
  (with-open-file (s (pathname fname)
		     :direction :output
		     :if-exists :supersede)
    (let ((the-lang-stream (make-instance 'l-readable-stream :stream s))
	  (*print-case* :downcase)
	  (objs (if (listp obj-list) obj-list (list obj-list))))

           
      (format s "(in-package :langband)~2%")
      (format s "(setf *save-hanger* nil)~2%")
      (dolist (i objs)
	(format s "(push ~%")
	(save-object i the-lang-stream 0)
	(format s " *save-hanger*)~2%"))
      )))

(defmethod do-save ((type (eql :binary)) fname obj-list)

  (bt:with-binary-file (s (pathname fname)
			  :direction :output
			  :if-exists :supersede)
    
    (let ((bt:*endian* :little-endian)
	  (the-lang-stream (make-instance 'l-binary-stream :stream s))
	  (objs (if (listp obj-list) obj-list (list obj-list))))

      
      ;;      (warn "saving binary with endian ~a" bt:*endian*)
      (dolist (i objs)
	(save-object i the-lang-stream 0))
      
      )))

(defmethod %save-obj (obj fname)
  (with-open-file (s (pathname fname)
		     :direction :output
		     :if-exists :supersede)
    (print obj s)))

(defmethod do-load ((type (eql :readable)) fname obj-type)
  (declare (ignore obj-type))
  
  (let ((*save-hanger* nil))
    (load fname)

    *save-hanger*))


(defmethod do-load ((type (eql :binary)) fname obj-type-list)

  (bt:with-binary-file (s (pathname fname)
			  :direction :input)
    
    (let ((bt:*endian* :little-endian)
	  (the-lang-stream (make-instance 'l-binary-stream :stream s))
	  (objs (if (listp obj-type-list) obj-type-list (list obj-type-list))))

      (loop for obj-type in objs
	    collecting (load-object obj-type the-lang-stream))
      )))

(defun save-the-game (var-obj player level &key (fname +readable-save-file+) (format :readable))
  "Tries to save the game."

  (do-save format fname (list var-obj player level))
  t)
