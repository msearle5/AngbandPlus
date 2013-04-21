;;; -*- Mode: Lisp; Syntax: Common-Lisp; Package: org.langband.engine -*-

#|

DESC: actions.lisp - various actions from the kbd/player
Copyright (c) 2000-2002 - Stig Erik Sand�

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

|#

(in-package :org.langband.engine)


(defun move-player! (dungeon player direction)
  "Moves the player, in a direction, if possible.
The direction is a number from the keypad."

  (let* ((var-obj *variant*)
	 (cur-x (location-x player))
	 (cur-y (location-y player))
	 (wanted-x cur-x)
	 (wanted-y cur-y))

    ;; hack, move to variant later
    (when (get-attribute-value '<confusion> (player.temp-attrs player))
      (let ((new-dir (randint 9)))
	;; ultra-hack
	(when (eql new-dir 5)
	  (setf new-dir direction))
	(setf direction new-dir)))
 
    
    (case direction
      (9 (decf wanted-y) (incf wanted-x))
      (8 (decf wanted-y))
      (7 (decf wanted-y) (decf wanted-x))
      (6 (incf wanted-x))
      (5 nil)
      (4 (decf wanted-x))
      (3 (incf wanted-y) (incf wanted-x))
      (2 (incf wanted-y))
      (1 (incf wanted-y) (decf wanted-x))
      (otherwise
       (warn "Unknown direction ~s" direction)
       (return-from move-player! nil)
       ))

    #||
    (warn "Position ~a ~a has currently ~a,~a -> ~a" wanted-x wanted-y (cave-floor dungeon wanted-x wanted-y)
	  (cave-info dungeon wanted-x wanted-y)
	  (cave-floor-bold? dungeon wanted-x wanted-y))

    ||#

    (setf (player.energy-use player) +energy-normal-action+)
    
    (let ((monsters (cave-monsters dungeon wanted-x wanted-y)))

      ;; monsters to attack
      (cond (monsters
	     (attack-location! dungeon player wanted-x wanted-y)
	     )

	    ((is-closed-door? dungeon wanted-x wanted-y)
	     (open-door! dungeon wanted-x wanted-y))
	    
	    ;; something is in the way
	    ((not (cave-floor-bold? dungeon wanted-x wanted-y))
	     (print-message! "Cannot walk that way.."))

	    ;; default is just to move
	    (t
	     (swap-monsters! dungeon player
			     (location-x player)
			     (location-y player)
			     wanted-x
			     wanted-y)

	     )
	    ))

    (let ((new-x (location-x player))
	  (new-y (location-y player)))
      
      (when-bind (objs (cave-objects dungeon new-x new-y))
	(check-type objs item-table)
	(assert (plusp (items.cur-size objs)))
	(let ((obj (item-table-find objs 0)))
	  (check-type obj active-object)
	  (format-message! "You see ~a."
			   (with-output-to-string (s)
			     (write-obj-description var-obj obj s)))
	  ))
      
      (bit-flag-add! *update* +pl-upd-update-view+)

      
      ;; hack
      (apply-possible-coord-trigger dungeon new-x new-y))

    player))


(defun search-area! (dungeon player)
  "Searches nearby grids."

  (let ((chance (skills.searching (player.skills player)))
	(x (location-x player))
	(y (location-y player))
	(ddx-ddd *ddx-ddd*)
	(ddy-ddd *ddy-ddd*))
    
    (check-type chance fixnum)
    
    (incf (player.energy-use player) +energy-normal-action+)
    
    ;; hack, improve with diagonal later
    (loop for i from 0 below +normal-direction-number+
	  for cur-x = (+ x (svref ddx-ddd i))
	  for cur-y = (+ y (svref ddy-ddd i))
	  do
	  (when (< (random 100) chance)
	    (let* ((coord (cave-coord dungeon cur-x cur-y))
		   (floor (coord.floor coord))
		   (decor (coord.decor coord)))
	      
	      (when (typep decor 'active-trap)
		(make-trap-visible decor dungeon cur-x cur-y)
		(print-message! "You have found a trap."))
	      
	      (when (= floor +floor-secret-door+)
		(print-message! "You found a secret door.")
		(place-closed-door! dungeon cur-x cur-y)
		(light-spot! dungeon cur-x cur-y))
	  
	      ;; add more here, traps, chests.. 
	      )))
    ))


(defun change-depth! (dungeon player &key (direction :down) (amount 1) type)

  (let* ((cur-depth (player.depth player))
	 (wanted-depth (ecase direction
			 (:up (- cur-depth amount))
			 (:down (+ cur-depth amount)))))

    (cond ((minusp wanted-depth)
	   (setf wanted-depth 0))
	  ((> wanted-depth (variant.max-depth *variant*))
	   (setf wanted-depth (variant.max-depth *variant*))))
    
    (setf (player.depth player) wanted-depth
	  (player.energy-use player) +energy-normal-action+ 
	  (dungeon.depth dungeon) wanted-depth)
    
    (when (> wanted-depth (player.max-depth player))
      (setf (player.max-depth player) wanted-depth))
    
    
    (setf (player.leaving-p player) type)

    (bit-flag-add! *redraw* +print-map+ +print-basic+)
    (bit-flag-add! *update* +pl-upd-update-view+)  
    
    t))

(defun use-stair! (dungeon player dir)
  "Uses a stair in direction DIR if the player PL
is above a stair.  DIR can be :UP or :DOWN"

  (let* ((depth (player.depth player))
	 (x (location-x player))
	 (y (location-y player))
	 (feat (cave-floor dungeon x y))
	 (leaving-sym nil))
    (declare (type u-16b depth x y feat))
    (case dir
      (:up
       (unless (= +floor-less+ feat)
	 (return-from use-stair! nil))
       
       (if (= depth 0)
	   (error "Cannot go upstairs from level 0")
	   (decf depth))
       (setf leaving-sym :up-stair))
      
      (:down
       (unless (= +floor-more+ feat)
	 (return-from use-stair! nil))
       
       (incf depth)
       (setf leaving-sym :down-stair))
      
      (otherwise
       (lang-warn "Unknown direction for stair-use ~a" dir)
       (return-from use-stair! nil)
       ))



    (change-depth! dungeon player :direction dir :amount 1 :type leaving-sym)
    

    t))

(defun pick-up-from-floor! (dungeon player)
  "Tries to pick up from floor.  Should be implemented with selection from
a list if more items occupy the same place."
  
  (let* ((var-obj *variant*)
	 (x (location-x player))
	 (y (location-y player))
	 (objs (cave-objects dungeon x y)))

    (if (not objs)
	(progn
	  (lang-warn "Nothing on floor.")
	  nil)

	(loop
	 (let ((removed-obj (item-table-remove! objs 0)))
	   (unless removed-obj
	       (return-from pick-up-from-floor! nil))

	   ;; hackish, move to variant later
	   (let ((removed-type (object.the-kind (aobj.kind removed-obj))))
	   (cond ((eq removed-type '<money>)
		  ;; are we gold?
		  (incf (player.gold player) (aobj.number removed-obj))
		  ;; redraw left frame
		  (bit-flag-add! *redraw* +print-basic+)
		  
		  (when (= 0 (items.cur-size objs))
		    (setf (cave-objects dungeon x y) nil)))
		 

		 (t
		  (let* ((backpack (aobj.contains (player.inventory player)))
			 (retval (item-table-add! backpack removed-obj)))

		    (cond (retval
			   (format-message! "You pick up ~a."
					    (with-output-to-string (s)
					      (write-obj-description var-obj removed-obj s)))
			   ;; succesful
			   (when (= 0 (items.cur-size objs))
			     (setf (cave-objects dungeon x y) nil)))

			  (t 
			   ;; not succesful.. put it back
			   (item-table-add! objs removed-obj)
			   (print-message! "No room in backpack.")
			   (return-from pick-up-from-floor! nil)))))
		 ))

	   (unless (cave-objects dungeon x y)
	     (return-from pick-up-from-floor! nil))))

	)))


(defun interactive-drop-item! (dungeon player)
  "Drop some inventory"

  (when-bind (selection (with-new-screen ()
			  (select-item dungeon player '(:backpack :equip)
				       :prompt "Drop item: "
				       :where :backpack)))

    (let* ((var-obj *variant*)
	   (the-table (get-item-table dungeon player (car selection)))
	   (removed-obj (item-table-remove! the-table (cdr selection))))

      (unless removed-obj
	(format-message! "Did not find selected obj ~a" selection)
	(return-from interactive-drop-item! nil))

      (when (and (typep removed-obj 'active-object)
		 (is-cursed? removed-obj))
	(print-message! "Hmmm, it seems to be cursed.")
	(item-table-add! the-table removed-obj) ;; put back
	(return-from interactive-drop-item! nil))

      (check-type removed-obj active-object)

      (drop-near-location! var-obj dungeon removed-obj (location-x player) (location-y player))
      (bit-flag-add! *update* +pl-upd-bonuses+ +pl-upd-mana+ +pl-upd-torch+)
      ;;(item-table-add! (get-item-table dungeon player :floor) removed-obj)
      t)))
			     


(defun %put-obj-in-cnt (dungeon player cnt obj)
  (let* ((the-table (if (typep cnt 'item-table) cnt (get-item-table dungeon player cnt)))
	 (back-to-inventory (item-table-add! the-table obj)))
    (unless back-to-inventory
      ;; drop to floor
      (item-table-add! (get-item-table dungeon player :floor)
		       obj))
    ))

;;(trace %put-obj-in-cnt)

(defun interactive-wear-item! (dungeon player)
  "Puts something in an equipment-slot."

  (when-bind (selection (with-new-screen ()
			  (select-item dungeon player '(:backpack :floor)
				       :prompt "Wear item"
				       :where :backpack)))
    
    
    (let* ((the-table (get-item-table dungeon player (car selection)))
	   (removed-obj (item-table-remove! the-table (cdr selection) :only-single-items t))
	   (other-obj nil))
;;      (warn "Removed ~a" removed-obj)

      (unless removed-obj ;; we need to get an object, if not return!
	(return-from interactive-wear-item! nil))

      ;; another obj?
      (setf other-obj (item-table-add! (get-item-table dungeon player :equip) removed-obj))
;;	  (warn "Adding to equip gave: ~a" other-obj)

      (when (eq other-obj nil)
	;; something screwed up, put object back
	(item-table-add! the-table removed-obj)
	(return-from interactive-wear-item! nil))

      (when (is-cursed? removed-obj)
	(print-message! "Oops! It feels deathly cold!"))
      
      (cond ((typep other-obj 'active-object)
	     ;; an object was returned
	     (%put-obj-in-cnt dungeon player :backpack other-obj)
	     (bit-flag-add! *update* +pl-upd-bonuses+ +pl-upd-mana+ +pl-upd-torch+))
	    
	    (t
	     ;; succesful and nothing returned.. do nothing, except waste energy
	     (incf (player.energy-use player) +energy-normal-action+)
	     ;; hack, fix later
	     
	     (bit-flag-add! *update* +pl-upd-bonuses+ +pl-upd-mana+ +pl-upd-torch+)
	     ;; add window-stuff
	     ))
      )))

(defmethod apply-usual-effects-on-used-object! (dungeon player obj)
  (declare (ignore dungeon))

  (let* ((okind (etypecase obj
		 (active-object (aobj.kind obj))
		 (object-kind obj)))
	 (gvals (object.game-values okind)))
  
  ;; we have tried object
  ;;(tried-object)
  
    (when (is-eatable? player obj)
      (alter-food! player (+ (player.food player)
			 (gval.food-value gvals))))
    
    
    t))


(defun interactive-use-item! (dungeon player
			      &key
			      (need-effect nil)
			      (selection-function nil)
			      (prompt "Use item?")
			      (which-use :use)
			      (limit-from '(:backpack :floor)))
  
  "Tries to use an item."


  (assert (consp limit-from))
  (assert (stringp prompt))

  (labels ((has-obj-effect (obj effect)
	     (find effect (object.effects (aobj.kind obj)) :key #'effect-entry-type))
	   (allowed-object (obj)
	     ;;(warn "checking ~s vs ~s, and have effects ~s" obj need-effect (object.effects (aobj.kind obj)))
	     (cond ((eq need-effect nil)
		    t)
		   ((consp need-effect)
		    (dolist (x need-effect)
		      (when (has-obj-effect obj x)
			(return-from allowed-object t))))
		   (t
		    (warn "Unknown need-effect value ~s" need-effect)
		    nil))))
  
  (let ((variant *variant*)
	(selection (with-new-screen ()
		     (select-item dungeon player limit-from
				  :prompt prompt
				  :where (first limit-from)
				  :selection-function (if selection-function
							  selection-function
							  #'allowed-object)
				  ))))

    (unless (and selection (consp selection))
      (return-from interactive-use-item! nil))

    
;;    (warn "Selected ~s for use" selection)
    (let* ((the-table (get-item-table dungeon player (car selection)))
	   (removed-obj (item-table-remove! the-table (cdr selection))))
      
;;      (warn "Removed ~a" removed-obj)
      
      (unless (and removed-obj (typep removed-obj 'active-object))
	(return-from interactive-use-item! nil))
      
;;      (warn "Will ~a ~s" prompt removed-obj)
	  
      (let ((retval (use-object! variant dungeon player removed-obj :which-use which-use)))
;;	(warn "use returned ~s" retval)
	(cond ((or (eq retval nil) ;; didn't use the object for some reason..
		   (eq retval :not-used))
	       (%put-obj-in-cnt dungeon player the-table removed-obj))
	      
	      ((eq retval :still-useful) ;; we should keep it
	       (let ((back-obj (%put-obj-in-cnt dungeon player the-table removed-obj)))
		 ;; add energy use
		 (apply-usual-effects-on-used-object! dungeon player removed-obj)
		 back-obj))
	      
	      ((eq retval :used) ;; we have used the item
	       ;; decrement number
	       (when (> (aobj.number removed-obj) 1)
		 (decf (aobj.number removed-obj))
		 (%put-obj-in-cnt dungeon player the-table removed-obj))
;;	       (warn "used object ~a" removed-obj)
	       (apply-usual-effects-on-used-object! dungeon player removed-obj)
	       nil)
	      
	      (t
	       (warn "Fell through on use return-value: ~s" retval)))
	
	))
      )))


(defun open-door! (dungeon x y)
  "hackish, fix me later.."
  (when (is-closed-door? dungeon x y)
    (setf (cave-floor dungeon x y) +floor-open-door+)
    (light-spot! dungeon x y)))

(defun close-door! (dungeon x y)
  "hackish, fix me later.."
  (when (is-open-door? dungeon x y)
    (setf (cave-floor dungeon x y) +floor-door-head+)
    (light-spot! dungeon x y)))

(defun bash-door! (dungeon x y)
  "hackish, fix me later.."
  ;; add paralysis, hitpoints, ...
;;  (warn "trying to bash door")
  (when (is-closed-door? dungeon x y)
    (when (= 1 (random 4)) ;; 1/4 chance
      (setf (cave-floor dungeon x y) +floor-broken-door+)
      (light-spot! dungeon x y))))


(defun interactive-door-operation! (dungeon player operation)

  (block door-operation

    (let ((check-predicate nil)
	  (operation-fun nil))
      
    (ecase operation
      (:open (setf check-predicate #'is-closed-door?
		   operation-fun #'open-door!))
      (:close (setf check-predicate #'is-open-door?
		    operation-fun #'close-door!))
      (:bash (setf check-predicate #'is-closed-door?
		   operation-fun #'bash-door!))
      (:jam
       (warn "Jamming is not properly implemented yet.")
       (return-from door-operation nil)))

;;    (warn "going ~s ~s ~s" operation check-predicate operation-fun)
     
    (let* ((x (location-x player))
	   (y (location-y player))
	   (ddx-ddd *ddx-ddd*)
	   (ddy-ddd *ddy-ddd*))
      

      ;; first check if there is just one door, then we operate on it

      (let ((count 0))
	(loop for i from 0 below +normal-direction-number+
	      for test-x = (+ x (svref ddx-ddd i))
	      for test-y = (+ y (svref ddy-ddd i))
	      do
	      (when (funcall check-predicate dungeon test-x test-y)
		(incf count)))

	(cond ((= count 1)
	       (loop for i from 0 below +normal-direction-number+
		     for test-x = (+ x (svref ddx-ddd i))
		     for test-y = (+ y (svref ddy-ddd i))
		     do
		     (when (funcall check-predicate dungeon test-x test-y)
		       (funcall operation-fun dungeon test-x test-y)
		       (return-from door-operation t))))
	      ((= count 0)
	       ;;(print-message! "No doors near you.!")
	       (return-from door-operation t))
	      ))
      
      ;; then we need to find which door
      (let ((dir (%read-direction)))
	(cond ((integerp dir)
	       (let ((new-x (+ x (aref *ddx* dir)))
		     (new-y (+ y (aref *ddy* dir))))
		 ;;(warn "Checking ~s (~s,~s) from (~s,~s)" dir new-x new-y x y)
		 (if (funcall check-predicate dungeon new-x new-y)
		     (funcall operation-fun dungeon new-x new-y)
		     ;; maybe add message on failure?
		     nil)))

	      (t
	       nil)))
      ))
    ))

(defun disarm-trap! (dungeon x y)
  ;; hackish
  (let ((decor (cave-decor dungeon x y)))
    (when (and (typep decor 'active-trap)
	       (= (random 4) 1))
      (setf (cave-decor dungeon x y) nil)
      (setf (dungeon.decor dungeon) (delete decor (dungeon.decor dungeon)))
      (print-message! "You disarm the trap.")
      (light-spot! dungeon x y)
      t)))

(defun is-visible-trap? (dungeon x y)
  (let ((decor (cave-decor dungeon x y)))
    (and (typep decor 'active-trap)
	 (decor.visible? decor))))

   

(defun interactive-trap-operation! (dungeon player operation)

  (block trap-operation

    (let ((check-predicate nil)
	  (operation-fun nil))
      
    (ecase operation
      (:disarm (setf check-predicate #'is-visible-trap?
		     operation-fun #'disarm-trap!)))
    
    (let* ((x (location-x player))
	   (y (location-y player))
	   (ddx-ddd *ddx-ddd*)
	   (ddy-ddd *ddy-ddd*))
      
      
      ;; first check if there is just one trap, then we operate on it
      
      (let ((count 0))
	(loop for i from 0 below +normal-direction-number+
	      for test-x = (+ x (svref ddx-ddd i))
	      for test-y = (+ y (svref ddy-ddd i))
	      do
	      (when (funcall check-predicate dungeon test-x test-y)
		(incf count)))

	(cond ((= count 1)
	       (loop for i from 0 below +normal-direction-number+
		     for test-x = (+ x (svref ddx-ddd i))
		     for test-y = (+ y (svref ddy-ddd i))
		     do
		     (when (funcall check-predicate dungeon test-x test-y)
		       (funcall operation-fun dungeon test-x test-y)
		       (return-from trap-operation t))))
	      ((= count 0)
	       ;;(print-message! "No doors near you.!")
	       (return-from trap-operation t))
	      ))
      
      ;; then we need to find which trap
      (let ((dir (%read-direction)))
	(cond ((integerp dir)
	       (let ((new-x (+ x (aref *ddx* dir)))
		     (new-y (+ y (aref *ddy* dir))))
		 ;;(warn "Checking ~s (~s,~s) from (~s,~s)" dir new-x new-y x y)
		 (if (funcall check-predicate dungeon new-x new-y)
		     (funcall operation-fun dungeon new-x new-y)
		     ;; maybe add message on failure?
		     nil)))
	      
	      (t
	       nil)))
      ))
    ))
