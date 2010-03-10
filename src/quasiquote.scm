(define make-tagged-list
  (lambda (elem tag)
    (cons tag (cons elem '()))))

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
        (if (eqv? (car lst) tag)
            (= (length lst) len)
            #f)
        #f)))

(define quasiquoted?
  (lambda (x)
    (list-tagged? x 2 'quasiquote)))

(define unquoted?
  (lambda (x)
    (list-tagged? x 2 'unquote)))

(define unquoted-splicing?
  (lambda (x)
    (list-tagged? x 2 'unquote-splicing)))

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

(define eval-quasiquote
  (lambda (pat env)
    (qq-one (quasiquote-text pat) 0 env)))

(define qq-one
  (lambda (pat level env)
    (if (null? pat)
        pat
        (if (not (pair? pat))
            pat
            (if (quasiquoted? pat)
                (make-quasiquote
                 (qq-one (quasiquote-text pat)
                         (+ level 1)
                         env))
                (if (unquoted? pat)
                    (if (= level 0)
                        (eval (unquote-text pat) env)
                        (make-unquote
                         (qq-one (unquote-text pat)
                                 (- level 1)
                                 env)))
                    (if (unquoted-splicing? pat)
                        (error)
                        (qq-list pat level env))))))))
(define qq-list
  (lambda (pat level env)
    ;; these helpers are basically here because we don't have let
    (define helper
      (lambda (elem rest)
        (if (quasiquoted? elem)
            (cons (make-quasiquote
                   (qq-one (quasiquote-text elem)
                           (+ level 1)
                           env))
                  rest)
            (if (unquoted? elem)
                (if (= level 0)
                    (cons (eval (unquote-text elem) env)
                          rest)
                    (cons (make-unquote
                           (qq-one (unquote-text elem)
                                   (- level 1)
                                   env))
                          rest))
                (if (unquoted-splicing? elem)
                    (if (= level 0)
                        (append (eval (unquote-splicing-text elem)
                                      env)
                                rest)
                        (cons (make-unquote-splicing
                               (qq-one (unquote-splicing-text elem)
                                       (- level 1)
                                       env))
                              rest))
                    (helper2 elem rest (qq-list elem level env)))))))
    (define helper2
      (lambda (elem rest value)
        (if (eq? value elem)
            (if (eq? rest (cdr pat))
                pat
                (cons value rest))
            (cons value rest))))
    (if (null? pat)
        pat
        (if (not (pair? pat))
            pat
            ;; pat is a pair
            (if (quasiquoted? pat)
                (make-quasiquote (qq-one (quasiquote-text pat)
                                         (+ level 1)
                                         env))
                (if (unquoted? pat)
                    (if (= level 0)
                        (eval (unquote-text pat) env)
                        (make-unquote (qq-one (unquote-text pat)
                                              (- level 1)
                                              env)))
                    (if (unquoted-splicing? pat)
                        (error)
                        (helper (car pat)
                                (qq-list (cdr pat) level env)))))))))