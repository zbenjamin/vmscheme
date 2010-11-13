(define (call/cc proc)
  (%call/cc (lambda (k)
              (proc (lambda (val) (invoke-continuation k val))))))
