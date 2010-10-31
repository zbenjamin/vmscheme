(define (initial-repl)
  (define (eval-each lst)
    (if (= (length lst) 1)
        (eval (car lst) (the-global-environment))
        (begin
          (eval (car lst) (the-global-environment))
          (eval-each (cdr lst)))))
  (display "> ")
  (let ((val (read)))
    (if (not (null? val))
        (begin
          (display (eval-each val))
          (display "\n"))))
  (initial-repl))
