(defvar kern-mode-hook nil)

(defvar kern-mode-map
  (let ((kern-mode-map (make-keymap)))
    (define-key kern-mode-map "\C-j" 'newline-and-indent)
    (define-key kern-mode-map [tab] 'kern-insert-tab)
    ;;(define-key kern-mode-map "\r" 'kern-newline)
    (define-key kern-mode-map "\r" 'newline)
    (define-key kern-mode-map (kbd "C-M-n") 'next-measure)
    (define-key kern-mode-map (kbd "C-M-p") 'previous-measure)
    (define-key kern-mode-map (kbd "C-M-m") 'go-to-measure)
    (define-key kern-mode-map (kbd "C-M-h") 'show-header)
    kern-mode-map)
  "Keymap for KERN major mode")

(add-to-list 'auto-mode-alist '("\\.krn\\'" . kern-mode))

(defun end-of-line-p ()
  (or (null (char-after)) (eql (char-after) 10)))

(defun blank-line-p ()
  (and (eql (current-column) 0)
       (end-of-line-p)))

(defun next-measure ()
  (interactive)
  (when (re-search-forward "^=" nil t)
    (backward-char 1)
    (next-line)))

(defun previous-measure ()
  (interactive)
  (previous-line)
  (when (re-search-backward "^=" nil t)
    (beginning-of-line)
    (next-line)))

(defun show-header ()
  (interactive)
  (save-excursion
    (goto-char (point-min)) ; beginnig of line
    (search-forward-regexp "^\\*\\*.*$" nil t)
    (beginning-of-line)
    (kill-line)
    (yank)
    (message (first kill-ring))))

(defun go-to-measure ()
  (interactive)
  (let ((m (read-from-minibuffer "measure #: ")))
    (if (re-search-forward (concat "^=" m) nil t)
        (progn
          (backward-char 1)
          (next-line))
        (progn
          (re-search-backward (concat "^=" m) nil t)
          (next-line)))))

(defun copy-line ()
  (interactive)
  (copy-region-as-kill (line-beginning-position) (line-end-position)))

(defun kern-newline ()
  (interactive "*")
  (unless (= (line-number-at-pos) 1)
    (save-excursion
      (previous-line)
      (copy-region-as-kill (point) (line-end-position)))
    (loop for x from 1 to (count ?\t (first kill-ring)) do
          (insert "\t."))
    (delete-horizontal-space t))
  (newline))

(defun kill-column ()
  (interactive)
  (search-backward-regexp "^\\*\\*")
  (let ((p (point))
        (e (search-forward-regexp "^\\*-\t")))
    (kill-rectangle p e)))

(defun newline-insert-measure ()
  (interactive)
  (let (last-measure row)
    (save-excursion
      (copy-region-as-kill (line-beginning-position) (line-end-position))
      (setq row (first kill-ring)))
    (save-excursion
      (search-backward-regexp "=\\([0-9]+\\)$" nil t)
      (goto-char (match-beginning 1))
      (copy-region-as-kill (point) (line-end-position))
      (setq last-measure (first kill-ring)))
    (delete-horizontal-space t)
    (newline)
    (loop for x from 1 to (count ?\t row)
          with m = (read-from-string last-measure)
          do
          (insert (format "=%d\t" m))
          finally (insert (format "=%d" m)))))
  
(defun kern-insert-tab ()
  (interactive)
  (let ((last-char (char-before)))
    (cond ((blank-line-p)
           (message "You need to insert some code."))
          ((eql (char-after) 33)
           (if (search-forward "**" nil t)
               (backward-char 2)
               (if (search-forward "*" nil t)
                   (backward-char 1)))) ;; go to next **
          ((and (eql last-char 9) ; if is a tab
                (end-of-line-p))
           (message "You need to insert some code."))
          ;; TODO: implement last-column-p
          ;;((last-column-p)
          ;; (next-line)
          ;; (beginning-of-line))
          ((not (end-of-line-p))
           (re-search-forward "\t" nil t))
          (t (insert "\t")))))

(defgroup kern-faces nil
  "kern modes faces"
  :group 'kern
  :prefix "kern-")

(defvar kern-font-lock-keywords
  '(("^!.*" . 'kern-global-comment-face)
    ("^\\*\\*.*" . 'kern-exclusive-face)
    ("^\\*.*" . 'kern-interpretation-face)
    ("=[0-9]*[:!|]*\t?" . 'kern-bar-face)
    ("[0-9.]+[a-grA-G#n-]+" . 'kern-notes)
   ))

(defface kern-bar-face
    '((((class color) (background dark))
       (:background "dim gray" :foreground "yellow"))
      (((class color) (background light))
       (:foreground "red" :background "LightSteelBlue1"))
      (t (:italic t)))
    "Face used for bars"
    :group 'kern)

(defface kern-global-comment-face
    '((((class color) (background dark))
       (:foreground "Firebrick"))
      (((class color) (background light))
       (:foreground "Firebrick"))
      (t (:italic t)))
    "Face used for global comments"
    :group 'kern)

(defface kern-exclusive-face
    '((((class color) (background dark))
       (:foreground "Blue1"))
      (((class color) (background light))
       (:foreground "Blue1"))
      (t (:italic t)))
    "Face used for exclusive interpretation"
    :group 'kern)

(defface kern-interpretation-face
    '((((class color) (background dark))
       (:foreground "ForestGreen"))
      (((class color) (background light))
       (:foreground "ForestGreen"))
      (t (:italic t)))
    "Face used for interpretation"
    :group 'kern)

(defface kern-notes
    '((((class color) (background dark))
       (:foreground "Orange"))
      (((class color) (background light))
       (:foreground "dark orange"))
      (t (:italic t)))
    "Face used for notes and duration"
    :group 'kern)

;; (defvar kern-mode-syntax-table
;;   (let ((kern-mode-syntax-table (make-syntax-table)))
;;     (modify-syntax-entry ?%  "<" kern-mode-syntax-table)
;;     kern-mode-syntax-table)
;;   "Syntax table for kern-mode")

(defun kern-mode ()
  (interactive)
  (kill-all-local-variables)
  (use-local-map kern-mode-map)
  ;;(set-syntax-table kern-mode-syntax-table)
  (set (make-local-variable 'font-lock-defaults) '(kern-font-lock-keywords))
  (set (make-local-variable 'indent-line-function)
       'indent-to-left-margin)
  (setq indent-tabs-mode t)
  (setq major-mode 'kern-mode)
  (setq mode-name "KERN")
  (setq default-tab-width 12)
  (setq comment-start "!")
  (run-hooks 'kern-mode-hook))

(provide 'kern)
