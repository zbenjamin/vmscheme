(define null?
  (lambda (x) (= (object-type x) 0)))

(define boolean?
  (lambda (x) (= (object-type x) 1)))

(define integer?
  (lambda (x) (= (object-type x) 2)))

(define string?
  (lambda (x) (= (object-type x) 3)))

(define symbol?
  (lambda (x) (= (object-type x) 4)))

(define pair?
  (lambda (x) (= (object-type x) 5)))

(define procedure?
  (lambda (x) (= (object-type x) 6)))

(define primitive-procedure?
  (lambda (x) (= (object-type x) 7)))

(define environment?
  (lambda (x) (= (object-type x) 9)))

(define list?
  (lambda (x) (if (null? x)
             #t
             (if (pair? x)
                 (list? (cdr x))
                 #f))))

(define not
  (lambda (x)
    (if (eq? x #f)
        #t
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

(define map
  (lambda (func lst)
    (define helper
      (lambda (lst result)
        (if (null? lst)
            result
            (helper (cdr lst)
                    (cons (func (car lst)) result)))))
    (reverse (helper lst '()))))
