; (c) guenter.ebermann@htl-hl.ac.at

(define (not x)
  (if x #f #t))

(define (abs x)
  (if (< x 0)
      (- x)
      x))

(define (member-impl obj lst compare)
  (define (recur lst)
    (if (null? lst)
        #f
	(if (compare obj (car lst))
            lst
            (recur (cdr lst)))))
  (recur lst))

(define (memv obj lst)
  (member-impl obj lst eqv?))

(define (memq obj lst)
  (member-impl obj lst eq?))

(define (member obj lst)
  (member-impl obj lst equal?))

(define (map f lst)
  (if (null? lst)
      '()
      (cons (f (car lst))
            (map f (cdr lst)))))

(define (reverse lst)
  (define (recur lst acc)
    (if (null? lst)
        acc
        (recur (cdr lst) (cons (car lst) acc))))
  (recur lst '()))

(define (void) (if #f #f))

(define (for-each f lst)
  (if (null? lst)
      (void)
      ((lambda ()
        (f (car lst))
	(for-each f (cdr lst))))))

(define (equal? a b)
  (or (eqv? a b)
      (and (pair? a)
           (pair? b)
           (equal? (car a) (car b))
           (equal? (cdr a) (cdr b)))
      (and (string? a)
           (string? b)
           (string=? a b))))
