;;; -*- Mode: Lisp; Syntax: Common-Lisp; Package: org.langband.engine -*-

#|

DESC: building.lisp - code which deals with buildings
Copyright (c) 2000-2002 - Stig Erik Sand�

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

|#

(in-package :org.langband.engine)


(defmethod visit-house (level (house house))
  (declare (ignore level))
  ;;(warn "simple house..")
  )
  
(defmethod find-owner-for-house (level house &key var-obj selection)
  (declare (ignore level house var-obj selection))
  nil)

(defmethod activate-object ((obj house) &key (owner nil) (var-obj *variant*) (level *level*))
  "Wakes a house(-type) from slumber and returns a usable house."

  (when (and owner (typep owner 'owner))
    (setf (house.owner obj) owner))

  (when (eq (house.owner obj) :random)
    (let ((new-owner (find-owner-for-house level obj
					   :var-obj var-obj :selection :random)))
      (when (acceptable-owner? obj new-owner)
	(setf (house.owner obj) new-owner))))
  
  (cond ((acceptable-owner? obj)
	 (return-from activate-object obj))
	(t
	 (warn "House ~a does not have an owner." obj)
	 nil)))

(defun acceptable-owner? (house &optional (owner nil))
  "Returns t or nil."
  (let ((the-owner (if owner owner (house.owner house))))
    (when (and the-owner (or (typep the-owner 'owner)
			 (eq the-owner :player)))
      t)))

(defun establish-house& (var-obj house &key (house-key nil))
  "Establish a house as available type.  Returns NIL
on failure, and the house on success."

  (when (and var-obj house
	     (typep var-obj 'variant)
	     (typep house 'house))
    
    (let ((table (variant.house-types var-obj))
	  (house-id (if house-key
			house-key
			(house.id house))))

      (setf (gethash house-id table) house)
      
      house)))


(defun establish-owner& (var-obj owner)
  "Establish an owner as available. Returns NIL on
failure and owner on success."
  
  (when (and var-obj owner
	     (typep var-obj 'variant)
	     (typep owner 'owner))
    
    (let ((table (variant.house-owners var-obj))
	  (owner-id (owner.id owner)))
      
      (setf (gethash owner-id table) owner)
      
      owner)))


(defun get-house (id &optional (var-obj *variant*))
  "Returns a house-type (non-activated) or NIL."

  (when (and var-obj id
	     (typep var-obj 'variant))
    
    (let ((table (variant.house-types var-obj)))
      
      (gethash id table))))

(defun get-owner (id &optional (var-obj *variant*))
  "Returns an owner (non-activated) or NIL."
  
  (when (and var-obj id
	     (typep var-obj 'variant))
    
    (let ((table (variant.house-owners var-obj)))
      
      (gethash id table))))



(defmethod build-house! (level (house house) topleft-x topleft-y
			       &key
			       (door-feature nil)
			       (door-trigger nil)
			       )

  (when level
;;    (warn "building house ~a on level ~a at [~a,~a]" house level topleft-x topleft-y)

    (let* ((dungeon (level.dungeon level))
	   (y0 topleft-y)
	   (x0 topleft-x)
	   (y1 (- y0 (randint 3)))
	   (y2 (+ y0 (randint 3)))
	   (x1 (- x0 (randint 5)))
	   (x2 (+ x0 (randint 5))))

      (loop for y from y1 to y2 do
	    (loop for x from x1 to x2 do
		  (setf (cave-floor dungeon x y) +floor-perm-extra+)))
      

     ;; add doors
     (let ((tmp (random 4))
	   (x 0)
	   (y 0))

       ;; skip relocating annoying doors
       
       (case tmp
	 ;; bottom
	 (0 (setq y y2
		  x (rand-range x1 x2)))
	 ;; top
	 (1 (setq y y1
		  x (rand-range x1 x2)))
	 ;; right
	 (2 (setq y (rand-range y1 y2)
		  x x2))
	 ;; left
	 (3 (setq y (rand-range y1 y2)
		  x x1))
	 
	 (t
	  (warn "Fall-through in door placement")
	  (setq y y2
		x x2)))

       ;; time to place house number
       (when door-feature
	 (setf (cave-floor dungeon x y) door-feature))
       
       (when door-trigger
	 (setf (get-coord-trigger dungeon x y) door-trigger))
       
       ))

    house))

(defun define-house (id &key name (type 'house) number x-attr x-char owner (no-items nil))
  "defines a house and adds it to the appropriate table."
  
  (let ((var-obj *variant*)
	(house (make-instance type :id id :name name :owner owner
			      :x-attr x-attr :x-char x-char)))

    ;; hackish
    (unless no-items
      (setf (house.items house) (make-container 24 ;; fix later
						'items-in-house)))
    
    (establish-house& var-obj house)

    (when (and number (numberp number))
      ;; add to numbered position
      (establish-house& var-obj house :house-key number))
    
    house))


(defmethod item-table-print ((table items-in-house)
			     &key
			     show-pause
			     start-x start-y
			     print-selection
			     (store t))

  (declare (ignore print-selection))
  (let ((x (if start-x start-x 0))
	(y (if start-y start-y 6))
	(i 0))

    (flet ((iterator-fun (a-table key val)
	     (declare (ignore a-table key))
	     (let ((attr (get-text-colour val))
		   (desc (with-output-to-string (s)
			   (write-obj-description *variant* val s :store store))))
	       (c-prt! "" (- x 2) (+ i y))
	       (put-coloured-str! +term-white+ (format nil "~a) " (i2a i)) x (+ i y))
	       
	       (put-coloured-str! attr desc (+ x 4) (+ i y))
	       
	       (let* ((weight (object.weight (aobj.kind val)))
		      (full-pounds (int-/ weight 10))
		      (fractions (mod weight 10)))
		 (c-prt! (format nil "~3d.~d lb~a" full-pounds fractions
				 (if (> full-pounds 1)
				     "s"
				     ""))
			 61 (+ i y)))

	       (when store
		 (let ((price (get-price val store)))
		   (put-coloured-str! +term-white+ (format nil "~9d " price)
				      70 (+ i y))))

	       (incf i))))
      
      
      (item-table-iterate! table #'iterator-fun)
    
      (when show-pause
	(pause-last-line!))

      )))
