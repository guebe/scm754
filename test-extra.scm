; Scheme 754 Test Suite
; By Guenter Ebermann, 2026
; In the public domain

; These extra tests complement test.scm

;;; Syntax

; Objects

'MIssissiPPi ; 'MIssissiPPi

; Booleans

#T ; #t
#F ; #f

; Chars

#\  ; a space
; a newline char on next line
#\

(+ 1 (+ 2.5 (+ 3 (+ 4.5 (+ 5 (+ 6.25 (+ 7 (+ 8.75 (+ 9 (+ 10.5 (+ 11 (+ 12.25)))))))))))) ; 80.75

(define x '(1 2))
(set-car! x 3)
(set-cdr! x 4)
x ;(3 . 4)
