(define null?
  (lambda (x) (= (object-type x) 0)))

(define unspecific?
  (lambda (x) (= (object-type x) 1)))

(define boolean?
  (lambda (x) (= (object-type x) 2)))

(define integer?
  (lambda (x) (= (object-type x) 3)))

(define string?
  (lambda (x) (= (object-type x) 4)))

(define symbol?
  (lambda (x) (= (object-type x) 5)))

(define pair?
  (lambda (x) (= (object-type x) 6)))

(define procedure?
  (lambda (x) (= (object-type x) 7)))

(define primitive-procedure?
  (lambda (x) (= (object-type x) 8)))

(define environment?
  (lambda (x) (= (object-type x) 10)))

(define list?
  (lambda (x) (if (null? x)
             #t
             (if (pair? x)
                 (list? (cdr x))
                 #f))))

(define list (lambda x x))

(define not
  (lambda (x)
    (if (eq? x #f)
        #t
        #f)))

(define equal?
  (lambda (x y)
    (if (pair? x)
        (if (pair? y)
            (if (eqv? (car x) (car y))
                (equal? (cdr x) (cdr y))
                #f)
            #f)
        #f)))

(define append
  (lambda (lst1 lst2)
    (if (null? lst1)
        lst2
        (cons (car lst1)
              (append (cdr lst1) lst2)))))

(define length
  (lambda (lst)
    (define helper
      (lambda (lst result)
        (if (null? lst)
            result
            (if (not (pair? lst))
                (error)
                (helper (cdr lst) (+ 1 result))))))
    (helper lst 0)))

(define list-tail
  (lambda (lst k)
    (if (= k 0)
        lst
        (list-tail (cdr lst) (- k 1)))))

(define map
  (lambda (func lst)
    (define helper
      (lambda (lst result)
        (if (null? lst)
            result
            (helper (cdr lst)
                    (cons (func (car lst)) result)))))
    (reverse (helper lst '()))))

(define initial-repl
  (lambda ()
    (define eval-each
      (lambda (lst)
        (if (= (length lst) 1)
            (eval (car lst) (the-global-environment))
            ((lambda ()
              (eval (car lst) (the-global-environment))
              (eval-each (cdr lst)))))))
    (display "> ")
    (display (eval-each (read)))
    (display "\n")
    (initial-repl)))
