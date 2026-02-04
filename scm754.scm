; (c) guenter.ebermann@htl-hl.ac.at

(define (not x)
  (if x #f #t))

(define (abs x)
  (if (< x 0)
      (- x)
      x))

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
