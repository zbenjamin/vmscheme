(define make-cons
  (lambda (car cdr)
    (cons 'cons (cons car (cons cdr '())))))

(define make-append
  (lambda (lst1 lst2)
    (cons 'append (cons lst1 (cons lst2 '())))))

(define make-tagged-list
  (lambda (elem tag)
    (make-cons (cons 'quote (cons tag '()))
               (make-cons elem
                          '()))))

(define do-quote
  (lambda (elem)
    (cons 'quote (cons elem '()))))

(define make-quasiquote
  (lambda (elem)
    (make-tagged-list elem 'quasiquote)))

(define make-unquote
  (lambda (elem)
    (make-tagged-list elem 'unquote)))

(define make-unquote-splicing
  (lambda (elem)
    (make-tagged-list elem 'unquote-splicing)))

(define list-tagged?
  (lambda (lst len tag)
    (if (list? lst)
        (if (pair? lst)
            (if (eqv? (car lst) tag)
                (= (length lst) len)
                #f)
            #f)
        #f)))

(define quoted?
  (lambda (x)
    (list-tagged? x 2 'quote)))

(define quasiquoted?
  (lambda (x)
    (list-tagged? x 2 'quasiquote)))

(define unquoted?
  (lambda (x)
    (list-tagged? x 2 'unquote)))

(define unquoted-splicing?
  (lambda (x)
    (list-tagged? x 2 'unquote-splicing)))

(define quote-text
  (lambda (x)
    (car (cdr x))))

(define quasiquote-text
  (lambda (x)
    (car (cdr x))))

(define unquote-text
  (lambda (x)
    (car (cdr x))))

(define unquote-splicing-text
  (lambda (x)
    (car (cdr x))))

(define error
  (lambda ()
    ;; causes an error
    (0)))

(define transform-quasiquote
  (lambda (pat)
    (qq-one (quasiquote-text pat) 0)))

(define qq-one
  (lambda (pat level)
    (if (null? pat)
        (do-quote pat)
        (if (not (pair? pat))
            (do-quote pat)
            (if (quasiquoted? pat)
                (make-quasiquote
                 (qq-one (quasiquote-text pat)
                         (+ level 1)))
                (if (unquoted? pat)
                    (if (= level 0)
                        (unquote-text pat)
                        (make-unquote
                         (qq-one (unquote-text pat)
                                 (- level 1))))
                    (if (unquoted-splicing? pat)
                        (error)
                        (qq-list pat level))))))))
(define qq-list
  (lambda (pat level)
    ;; these helpers are basically here because we don't have let
    (define helper
      (lambda (elem rest)
        (if (quasiquoted? elem)
            (make-cons (make-quasiquote
                   (qq-one (quasiquote-text elem)
                           (+ level 1)))
                  rest)
            (if (unquoted? elem)
                (if (= level 0)
                    (make-cons (unquote-text elem)
                               rest)
                    (make-cons (make-unquote
                                (qq-one (unquote-text elem)
                                        (- level 1)))
                               rest))
                (if (unquoted-splicing? elem)
                    (if (= level 0)
                        (make-append (unquote-splicing-text elem)
                                     rest)
                        (make-cons (make-unquote-splicing
                                    (qq-one (unquote-splicing-text
                                             elem)
                                            (- level 1)))
                                   rest))
                    (helper2 elem rest (qq-list elem level)))))))
    (define helper2
      (lambda (elem rest value)
        (if (quoted? value)
            (if (eq? elem (quote-text value))
                (if (quoted? rest)
                    (if (eq? (quote-text rest) (cdr pat))
                        (do-quote pat)
                        (make-cons do-quote rest))
                    (make-cons value rest))
                (make-cons value rest))
            (make-cons value rest))))
    (if (null? pat)
        (do-quote pat)
        (if (not (pair? pat))
            (do-quote pat)
            ;; pat is a pair
            (if (quasiquoted? pat)
                (make-quasiquote (qq-one (quasiquote-text pat)
                                         (+ level 1)))
                (if (unquoted? pat)
                    (if (= level 0)
                        (unquote-text pat)
                        (make-unquote (qq-one (unquote-text pat)
                                              (- level 1))))
                    (if (unquoted-splicing? pat)
                        (error)
                        (helper (car pat)
                                (qq-list (cdr pat) level)))))))))
