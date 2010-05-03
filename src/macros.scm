;; The matcher is based on the matcher written by Hanson and Sussman
;; for the MIT course "Adventures in Advanced Symbolic Programming,"
;; which, in turn, was inspired by the idea from Hewitt's PhD thesis
;; (1969)

;; support functions

(define *var-tag* (list '*var-tag*))
(define *evar-tag* (list '*evar-tag*))
(define *repeat-tag* (list '*repeat-tag*))

(define macro:make-variable
  (lambda (name)
    (list *var-tag* name)))

(define macro:variable?
  (lambda (x)
    (%if (list? x)
        (%if (= (length x) 2)
            (eq? (car x) *var-tag*)
            #f)
        #f)))

(define macro:variable-name
  (lambda (x)
    (car (cdr x))))

(define macro:make-e-variable
  (lambda (name)
    (list *evar-tag* name)))

(define macro:e-variable?
  (lambda (x)
    (%if (list? x)
        (%if (= (length x) 2)
            (eq? (car x) *evar-tag*)
            #f)
        #f)))

(define macro:e-variable-name
  (lambda (x)
    (car (cdr x))))

(define macro:make-repeat
  (lambda (pattern)
    (list *repeat-tag* pattern)))

(define macro:repeat?
  (lambda (x)
    (%if (list? x)
        (%if (= (length x) 2)
            (eq? (car x) *repeat-tag*)
            #f)
        #f)))

(define macro:repeat-pattern
  (lambda (x)
    (car (cdr x))))

(define macro:extract-variables
  (lambda (pat)
    (define loop
      (lambda (pat result)
        (%if (pair? pat)
            (%if (macro:variable? (car pat))
                (loop (cdr pat)
                      (cons (macro:variable-name (car pat))
                            result))
                (%if (macro:e-variable? (car pat))
                    (loop (cdr pat)
                          (cons (macro:e-variable-name (car pat))
                                result))
                    (%if (pair? (car pat))
                        (loop (cdr pat)
                              (append
                               (macro:extract-variables (car pat))
                               result))
                        (loop (cdr pat) result))))
            result)))
    (loop pat '())))

;; combinators for matching
(define match:eqv
  (lambda (const)
    (lambda (data dict succeed)
      (%if (pair? data)
          (%if (eqv? (car data) const)
              (succeed dict 1)
              #f)
          #f))))

(define match:variable
  (lambda (var)
    (lambda (data dict succeed)
      (%if (pair? data)
          ((lambda ()
            (define vcell (macro:lookup var dict))
            (%if vcell
                (%if (equal? (macro:value vcell) (car data))
                    (succeed dict 1)
                    #f)
                (succeed (macro:bind var (car data) dict) 1))))
          #f))))

(define match:e-variable
  (lambda (var)
    (lambda (data dict succeed)
      (%if (pair? data)
          ((lambda ()
             (define vcell (macro:lookup var dict))
             (%if vcell
                 (succeed (macro:bind var
                                      (append (macro:value vcell)
                                              (list (car data)))
                                      dict)
                          1)
                 (succeed (macro:bind var (list (car data)) dict)
                          1))))
          #f))))

(define match:repeat
  (lambda (combinator)
    (lambda (data dict succeed)
      (define loop
        (lambda (data dict n)
          (%if (null? data)
              (succeed dict n)
              (%if (pair? data)
                  (combinator data dict
                              (lambda (newdict k)
                                (loop (list-tail data k)
                                      newdict
                                      (+ n k))))
                  #f))))
      (loop data dict 0))))

;; we don't have to deal with any backtracking because the only
;; matcher type that consumes more than one element is repeat, and
;; it can only occur at the end of a list
(define match:list
  (lambda combinators
    (define list-match
      (lambda (data dict succeed)
        (define loop
          (lambda (data matchers dict)
            (%if (pair? matchers)
                ((car matchers) data dict
                 (lambda (newdict n)
                   (loop (list-tail data n) (list-tail matchers 1)
                         newdict)))
                (%if (null? matchers)
                    (%if (pair? data)
                        #f
                        (succeed dict 1))
                    ;; improper list, which we don't handle yet
                    #f))))
        (loop (car data) combinators dict)))
    list-match))

;; dictionary support

(define assq
  (lambda (key alist)
    (%if (null? alist)
        #f
        (%if (pair? alist)
            (%if (eq? (car (car alist)) key)
                (car alist)
                (assq key (cdr alist)))
            (error "not an alist")))))

(define macro:bind
  (lambda (var data dict)
    (cons (list var data) dict)))

(define macro:lookup
  (lambda (var dict)
    (assq var dict)))

(define macro:value
  (lambda (vcell)
    (car (cdr vcell))))

;; compiler for matcher
(define match:->combinators
  (lambda (pattern)
    (define compile
      (lambda (pattern)
        (%if (macro:variable? pattern)
            (match:variable (macro:variable-name pattern))
            (%if (macro:e-variable? pattern)
                (match:e-variable (macro:e-variable-name pattern))
                (%if (macro:repeat? pattern)
                    (match:repeat (compile
                                   (macro:repeat-pattern pattern)))
                    (%if (list? pattern)
                        (apply match:list (map compile pattern))
                        (match:eqv pattern)))))))
    (compile pattern)))

;; substitution combinators

(define subst:eqv
  (lambda (const)
    (lambda (dict succeed)
      (succeed dict (list const)))))

(define subst:variable
  (lambda (var)
    (lambda (dict succeed)
      (succeed dict (list (macro:value (macro:lookup var dict)))))))

(define subst:e-variable
  (lambda (var)
    (lambda (dict succeed)
      (define vcell (macro:lookup var dict))
      (%if vcell
          (%if (pair? (macro:value vcell))
              (succeed (macro:bind var (cdr (macro:value vcell)) dict)
                       (list (car (macro:value vcell))))
              (succeed dict '()))
          (succeed dict '())))))

(define subst:repeat
  (lambda (combinator)
    (lambda (dict succeed)
      (define loop
        (lambda (dict result)
          (combinator dict
                      (lambda (newdict items)
                        (%if (null? items)
                            (succeed newdict result)
                            (loop newdict (append result items)))))))
      (loop dict '()))))

(define subst:list
  (lambda combinators
    (lambda (dict succeed)
      (define loop
        (lambda (dict substituters result)
          (%if (pair? substituters)
              ((car substituters) dict
               (lambda (newdict items)
                 (loop newdict (cdr substituters)
                       (append result items))))
              (succeed dict (list result)))))
      (loop dict combinators '()))))

(define subst:->combinators
  (lambda (pattern)
    (define compile
      (lambda (pattern)
        (%if (macro:variable? pattern)
            (subst:variable (macro:variable-name pattern))
            (%if (macro:e-variable? pattern)
                (subst:e-variable (macro:e-variable-name pattern))
                (%if (macro:repeat? pattern)
                    (subst:repeat (compile
                                   (macro:repeat-pattern pattern)))
                    (%if (list? pattern)
                        (apply subst:list (map compile pattern))
                        (subst:eqv pattern)))))))
    (compile pattern)))

;; procedures to convert the R5RS pattern language to an internal
;; form.  For example,
;; (cond (else result1 result2 ...))
;;   =>
;; (cons (else ((*var-tag*) result1) ((*evar-tag*) result2)))

(define contains?
  (lambda (x lst)
    (%if (null? lst)
        #f
        (%if (pair? lst)
            (%if (eq? x (car lst))
                #t
                (contains? x (cdr lst)))
            #f))))

(define error
  (lambda (msg)
    (display "error: ")
    (display msg)
    (display "\n")
    (0)))

(define match:convert-pattern
  (lambda (pat literals)
    (macro:convert-pattern-helper
     pat
     #f
     #t
     (lambda (pat emode)
       (%if (contains? pat literals)
           pat
           (%if (symbol? pat)
               (%if emode
                   (macro:make-e-variable pat)
                   (macro:make-variable pat))
               pat))))))

(define subst:convert-pattern
  (lambda (pat variables)
    (macro:convert-pattern-helper
     pat
     #f
     #f
     (lambda (pat emode)
       (%if (contains? pat variables)
           (%if emode
               (macro:make-e-variable pat)
               (macro:make-variable pat))
           pat)))))

(define macro:convert-pattern-helper
  (lambda (pat emode ellipsis-check atom-handler)
    (%if (null? pat)
        pat
        (%if (pair? pat)
            (%if (pair? (cdr pat))
                (%if (eq? (car (cdr pat)) '...)
                    (%if ellipsis-check
                        (%if (pair? (cdr (cdr pat)))
                            (error "bad context for '...'")
                            (cons (macro:make-repeat
                                   (macro:convert-pattern-helper
                                    (car pat)
                                    #t
                                    ellipsis-check
                                    atom-handler))
                                  (macro:convert-pattern-helper
                                   (cdr (cdr pat)) #f ellipsis-check
                                   atom-handler)))
                        (cons (macro:make-repeat
                               (macro:convert-pattern-helper
                                (car pat)
                                #t
                                ellipsis-check
                                atom-handler))
                              (macro:convert-pattern-helper
                               (cdr (cdr pat)) #f ellipsis-check
                               atom-handler)))
                    (cons (macro:convert-pattern-helper
                           (car pat) emode ellipsis-check
                           atom-handler)
                          (macro:convert-pattern-helper
                           (cdr pat) emode ellipsis-check
                           atom-handler)))
                (cons (macro:convert-pattern-helper
                       (car pat) emode ellipsis-check
                       atom-handler)
                      (macro:convert-pattern-helper
                       (cdr pat) emode ellipsis-check
                       atom-handler)))
            (atom-handler pat emode)))))

;; procedures for making macro transformers

(define macro:make-rule
  (lambda (match-pattern subst-pattern)
    (define matcher (match:->combinators match-pattern))
    (define substituter (subst:->combinators subst-pattern))
    (lambda (form)
      (matcher (list form)
               '()
               (lambda (dict n)
                 (substituter dict (lambda (d result) result)))))))

(define macro:clause->rule
  (lambda (match-pattern subst-pattern literals)
    (define new-match-pattern
      (match:convert-pattern match-pattern literals))
    (define new-subst-pattern
      (subst:convert-pattern
       subst-pattern
       (macro:extract-variables new-match-pattern)))
    (macro:make-rule new-match-pattern new-subst-pattern)))

(define macro:rules->transformer
  (lambda (rules)
    (lambda (form)
      (define loop
        (lambda (rules)
          (%if (null? rules)
              (error "bad syntax")
              ((lambda (val)
                 (%if val
                     val
                     (loop (cdr rules))))
               ((car rules) form)))))
      (loop rules))))

(define macro:clauses->transformer
  (lambda (clauses literals)
    (macro:rules->transformer
     (map (lambda (clause)
            (macro:clause->rule (car clause)
                                (car (cdr clause))
                                literals))
          clauses))))

(define *macro-table* '())

(define macro:add-macro!
  (lambda (name transformer)
    (set! *macro-table*
          (macro:bind name transformer *macro-table*))))

(define macro:find-and-transform
  (lambda (form)
    (define vcell (macro:lookup (car form) *macro-table*))
    (%if vcell
        ((macro:value vcell) form)
        #f)))

;; and, finally, the macro for defining new macros

(macro:add-macro! 'define-syntax
  (macro:clauses->transformer
   ;; we need the functions to be evaluated in this environment, not
   ;; the user environment
   (list (list '(define-syntax name
                  (syntax-rules () clause1 clause2 ...))
               (list macro:add-macro! ''name
                     (list macro:clauses->transformer
                           ''(clause1 clause2 ...) ''(name))))
         (list '(define-syntax name
                  (syntax-rules (lit1 lit2 ...) clause1 clause2 ...))
               (list macro:add-macro! ''name
                     (list macro:clauses->transformer
                           ''(clause1 clause2 ...)
                           ''(name lit1 lit2 ...)))))
   '(define-syntax syntax-rules)))
