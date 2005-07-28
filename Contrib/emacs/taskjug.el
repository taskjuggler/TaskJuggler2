;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;;  TaskJuggler Mode $Id$
;;;
;;;   Copyright (c) 2004 by Sean Dague (http://dague.net)
;;;
;;;   This program is free software; you can redistribute it and/or modify
;;;    it under the terms of the GNU General Public License as published by
;;;    the Free Software Foundation; either version 2 of the License, or
;;;    (at your option) any later version.
;;; 
;;;    This program is distributed in the hope that it will be useful,
;;;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;;;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;;    GNU General Public License for more details.
;;; 
;;;    You should have received a copy of the GNU General Public License
;;;    along with this program; if not, write to the Free Software
;;;    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;;;
;;;   This mode is based on the very good emacs mode tutorial
;;;   located at http://two-wugs.net/emacs/mode-tutorial.html created
;;;   by Scott Andrew Borton.  Thanks to him for creating such a well
;;;   structured tutorial
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defvar taskjug-mode-hook nil)

(defvar taskjug-mode-map
  (let ((taskjug-mode-map (make-keymap)))
    (define-key taskjug-mode-map "\C-j" 'newline-and-indent)
    taskjug-mode-map)
  "Keymap for TASKJUG major mode")

(add-to-list 'auto-mode-alist '("\\.tjp\\'" . taskjug-mode))
(add-to-list 'auto-mode-alist '("\\.tji\\'" . taskjug-mode))
(add-to-list 'auto-mode-alist '("\\.tjsp\\'" . taskjug-mode))

(defconst taskjug-font-lock-keywords-1
  (list
   '("\\<\\(flags\\|include\\|project\\|resource\\|task\\|scenario\\)\\>" . font-lock-keyword-face)
   '("\\('\\w*'\\)" . font-lock-variable-name-face))
  "Minimal highlighting expressions for Taskjug mode")

(defvar taskjug-font-lock-keywords taskjug-font-lock-keywords-1
  "Default highlighting expressions for TASKJUG mode")

(defvar taskjug-mode-syntax-table
  (let ((taskjug-mode-syntax-table (make-syntax-table)))
    
; This is added so entity names with underscores can be more easily parsed
    (modify-syntax-entry ?_ "w" taskjug-mode-syntax-table)
    (modify-syntax-entry ?# "<" taskjug-mode-syntax-table)
    (modify-syntax-entry ?\n ">" taskjug-mode-syntax-table)
    taskjug-mode-syntax-table)
  "Syntax table for taskjug-mode")

  
(defun taskjug-indent-line ()
  "Indent current line as TASKJUG code."
  (interactive)
  (beginning-of-line)
  (if (bobp)
      (indent-line-to 0)		   ; First line is always non-indented
    (let ((not-indented t) cur-indent)
      (if (looking-at "^[ \t]*}") ; If the line we are looking at is the end of a block, then decrease the indentation
          (progn
            (save-excursion
              (forward-line -1)
              (setq cur-indent (- (current-indentation) default-tab-width)))
            (if (< cur-indent 0) ; We can't indent past the left margin
                (setq cur-indent 0)))
        
        (save-excursion
          (while not-indented ; Iterate backwards until we find an indentation hint
            (forward-line -1)
            (if (looking-at "^.*}") ; This hint indicates that we need to indent at the level of the } token
                (progn
                  (setq cur-indent (current-indentation))
                  (setq not-indented nil))
              
              (if (looking-at "^.*{") ;This hint indicates that we need to indent an extra level
                  (progn
                    (setq cur-indent (+ (current-indentation) default-tab-width)) ; Do the actual indenting
                    (setq not-indented nil))
                (if (bobp)
                    (setq not-indented nil)))))))
      
      (if cur-indent
          (indent-line-to cur-indent)
        (indent-line-to 0))))) ; If we didn't see an indentation hint, then allow no indentation


(defun taskjug-mode ()
  "Major mode for editing TaskJuggler input files"
  (interactive)
  (kill-all-local-variables)
  (set-syntax-table taskjug-mode-syntax-table)
  (set (make-local-variable 'indent-line-function) 'taskjug-indent-line) 
  (use-local-map taskjug-mode-map)
  (set (make-local-variable 'font-lock-defaults) '(taskjug-font-lock-keywords))
  (setq major-mode 'taskjug-mode)
  (setq mode-name "TaskJuggler")
  (run-hooks 'taskjug-mode-hook))

(provide 'taskjug-mode)
