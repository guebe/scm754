; Scheme 754 Test Suite
; By Guenter Ebermann, 2026
; In the public domain

(define (fail result expected)
  (display "test failed: ")
  (newline)
  (display "got result:  ")
  (display result)
  (newline)
  (display "expected:    ")
  (display expected)
  (newline))

(define (test result expected)
  (if (not (equal? result expected))
      (fail result expected)))

; === Beginning of R7RS tests ===

; R7RS tests, 2.1 Identifiers

(test (symbol? '...) #t)
(test (symbol? '+) #t)
(test (symbol? '+soup+) #t)
(test (symbol? '<=?) #t)
(test (symbol? '->string) #t)
(test (symbol? 'a34kTMNs) #t)
(test (symbol? 'lambda) #t)
(test (symbol? 'list->vector) #t)
(test (symbol? 'q) #t)
(test (symbol? 'V17a) #t)
;(test (symbol? '|two words|) #t)
;(test (symbol? '|two\x20;words|) #t)
(test (symbol? 'the-word-recursion-has-many-meanings) #t)

; R7RS tests, 3.3. External representation

(test 28 #x1c)
(test '( 08 13 ) '(8 . (13 . ())))
(test (car '(+ 2 6)) '+)
(test (symbol? (car '(+ 2 6))) #t)
(test (car (cdr '(+ 2 6))) 2)
(test (number? (car (cdr '(+ 2 6)))) #t)
(test (car (cdr (cdr '(+ 2 6)))) 6)
(test (number? (car (cdr (cdr '(+ 2 6))))) #t)

; R7RS tests, 4.1.2 Literal expression

(test (quote a) 'a)
;(test (quote #(a b c)) '#(a b c))
(test (quote (+ 1 2)) '(+ 1 2))

(test 'a (quote a))
;(test '#(a b c) (quote #(a b c)))
(test '() (quote ()))
(test '(+ 1 2) (quote (+ 1 2)))
(test '(quote a) (quote (quote a)))
(test ''a (quote (quote a)))

(test '145932 145932)
(test '"abc" "abc")
;(test '# #)
;(test '#(a 10) #(a 10))
;(test '#u8(64 65) #u8(64 65))
(test '#t #t)

; R7RS tests, 6.2.2 Exactness

(test (= (/ 3 4) 0) #f) 
(test (or (not (= (* 0 +inf) (* 0 +inf))) (= (* 0 +inf) 0))  #t)

; R7RS tests, 6.2.4 Implementation extensions

(test (= -inf +inf) #f)
(test (+ 1.0 +inf) +inf)
(test (* 2.0 +inf) +inf)
(test (+ -1.0 +inf) +inf)
(test (* -2.0 +inf) -inf)
(test (+ 1.0 -inf) -inf)
(test (* 2.0 -inf) -inf)
(test (+ -1.0 -inf) -inf)
(test (* -2.0 -inf) +inf)
(test (= (+ -inf +inf) (+ -inf +inf)) #f)

(test (= +nan +nan) #f)
(test (= +nan -nan) #f)
(test (= -nan -nan) #f)
(test (= +nan 0.0) #f)
(test (= +nan -0.0) #f)
(test (= -nan 0.0) #f)
(test (= -nan -0.0) #f)
(test (= (+ +nan 1.0) (+ +nan 1.0)) #f)
(test (= (+ +nan -1.0) (+ +nan -1.0)) #f)
(test (= (+ -nan 1.0) (+ -nan 1.0)) #f)
(test (= (+ -nan -1.0) (+ -nan -1.0)) #f)
(test (= (* +nan 2.0) (* +nan 2.0)) #f)
(test (= (* +nan -2.0) (* +nan -2.0)) #f)
(test (= (* -nan 2.0) (* -nan 2.0)) #f)
(test (= (* -nan -2.0) (* -nan -2.0)) #f)

(test (eq? (- -0.0) +0.0) #t)
(test (eq? (- -0.0) 0.0) #t)
(test (eq? (- +0.0) -0.0) #t)
(test (eq? (- 0.0) -0.0) #t)
(test (eq? (- -0.0 +0.0) -0.0) #t)
(test (= -0.0 +0.0) #t)
(test (= -0.0 0.0) #t)
(test (= +0.0 0.0) #t)
(test (eq? +0.0 -0.0) #f)

; R7RS tests, 6.3 Booleans

(test #T #t)
(test #F #f)
(test '#f #f)

(test (if #t 't 'f) 't)
(test (if 0.0 't 'f) 't)
(test (if "a" 't 'f) 't)
(test (if "#\a" 't 'f) 't)
(test (if (cons "a" "b") 't 'f) 't)
(test (if #f 't 'f) 'f)

(test (equal? #f '()) #f)
(test (equal? #f 'nil) #f)
(test (equal? '() 'nil) #f)

(test (not #t) #f)
(test (not 3) #f)
;(test (not (list 3)) #f)
(test (not #f) #t)
(test (not '()) #f)
;(test (not (list)) #f)
(test (not 'nil) #f)

; R7RS tests, 6.4 Pairs and lists

(test '(a b c d e) '(a . (b . (c . (d . (e . ()))))))
(test '(a b c . d) '(a . (b . (c . d))))

(test (pair? '(a . b)) #t)
(test (pair? '(a b c)) #t)
(test (pair? '()) #f)
;(test (pair? '#(a b)) #f)

(test (cons 'a '()) '(a))
(test (cons '(a) '(b c d)) '((a) b c d))
(test (cons "a" '(b c)) '("a" b c))
(test (cons 'a 3) '(a . 3))
(test (cons '(a b) 'c) '((a b) . c))

(test (car '(a b c)) 'a)
(test (car '((a) b c d)) '(a))
(test (car '(1 . 2)) 1)
;(car '())

(test (cdr '((a) b c d)) '(b c d))
(test (cdr '(1 . 2)) 2)
;(cdr '())

(define f (cons 'not-a-constant-list '()))
(set-car! f 3)
(test (car f) 3)
(set-cdr! f 3)
(test (cdr f) 3)

; R7RS tests, 6.5 Symbols

(test (symbol? 'foo) #t)
(test (symbol? (car '(a b))) #t)
(test (symbol? "bar") #f)
(test (symbol? 'nil) #t)
(test (symbol? '()) #f)
(test (symbol? #f) #f)

; R7RS tests, 6.6. Characters

(test (char? #\a) #t); lower case letter
(test (char? #\A) #t); upper case letter
(test (char? #\() #t); left parenthesis
(test (char? #\ ) #t); the space character

; === End of R7RS tests ===
