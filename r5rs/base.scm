; http://www.schemers.org/Documents/Standards/R5RS/HTML/
(define-library (r5rs base)
   (begin
      ; ========================================================================================================
      ; Scheme
      ;
      ; Revised(3) Report on the Algorithmic Language Scheme
      ;                  Dedicated to the Memory of ALGOL 60
      ;
      ; ========================================================================================================
      ; There are some standard chapters sequence changed, due to more cleaner code:
      ;  chapters 4.2.2 (let, let*, letrec) and 4.2.3 (begin) goes before 4.1.5 due
      ;  to usings of this constructions in following definitins.

      ;;; Chapter 1
      ;;; Overview of Scheme
      ;; 1.1  Semantics
      ;; 1.2  Syntax
      ;; 1.3  Notation and terminology
      ; 1.3.1  Primitive, library, and optional features
      ; 1.3.2  Error situations and unspecified behavior
      (define-syntax syntax-error
         (syntax-rules (error)
            ((syntax-error . stuff)
               (error "Syntax error: " (quote stuff)))))

      ; 1.3.3  Entry format
      ; 1.3.4  Evaluation examples
      ; 1.3.5  Naming conventions
      
      
      ;;; Chapter 2
      ;;; Lexical conventions
      ;
      ;  This section gives an informal account of some of the lexical conventions used in writing Scheme
      ; programs. For a formal syntax of Scheme, see section 7.1.
      ;  Upper and lower case forms of a letter are never distinguished except within character and string
      ; constants. For example, Foo is the same identifier as FOO, and #x1AB is the same number as #X1ab.
      ;
      ;; 2.1  Identifiers
      ; lambda        q
      ; list->vector  soup
      ; +             V17a
      ; <=?           a34kTMNs
      ; the-word-recursion-has-many-meanings
      ;
      ;  Extended alphabetic characters may be used within identifiers as if they were letters. The
      ; following are extended alphabetic characters:
      ;
      ; ! $ % & * + - . / : < = > ? @ ^ _ ~
      ;

      ;; 2.2  Whitespace and comments
      ;; 2.3  Other notations


      ;;; Chapter 3
      ;;; Basic concepts
      ;; 3.1  Variables, syntactic keywords, and regions
      ;; 3.2  Disjointness of types
      ;; 3.3  External representations
      ;; 3.4  Storage model
      ;; 3.5  Proper tail recursion
      
      
      ;;; Chapter 4
      ;;; Expressions
      ;; 4.1  Primitive expression types
      ; 4.1.1  Variable references
      ; syntax:  <variable>
      
      
      ; 4.1.2  Literal expressions
      ; 4.1.3  Procedure calls
      ; 4.1.4  Procedures
      ; syntax:  (lambda <formals> <body>) 
      (define-syntax λ
         (syntax-rules () 
            ((λ . x) (lambda . x))))

      ; --- 4.2.2 and 4.2.3 must be declared before 4.1.5, 4.1.6, 4.2.1 due to language dependencies

      ; -------------------------
      ; 4.2.2  Binding constructs

      ; The three binding constructs let, let*, and letrec give Scheme a block structure, like Algol 60.
      ; The syntax of the three constructs is identical, but they differ in the regions they establish
      ; for their variable bindings. In a let expression, the initial values are computed before any of
      ; the variables become bound; in a let* expression, the bindings and evaluations are performed
      ; sequentially; while in a letrec expression, all the bindings are in effect while their initial
      ; values are being computed, thus allowing mutually recursive definitions.

      ; library syntax:  (letrec <bindings> <body>)
      (define-syntax letrec
         (syntax-rules (rlambda)
            ((letrec ((?var ?val) ...) ?body) (rlambda (?var ...) (?val ...) ?body))
            ((letrec vars body ...) (letrec vars (begin body ...)))))

      ; library syntax:  (letrec* ...) - extension
      (define-syntax letrec*
         (syntax-rules ()
            ((letrec () . body)
               (begin . body))
            ((letrec* ((var val) . rest) . body)
               (letrec ((var val))
                  (letrec* rest . body)))))

      ; library syntax:  (let <bindings> <body>)
      ;                  (let keyword <bindings> <body>) named let, from 4.2.4 Iteration
      (define-syntax let
            (syntax-rules ()
               ((let ((var val) ...) exp . rest) 
                  ((lambda (var ...) exp . rest) val ...))
               ((let keyword ((var init) ...) exp . rest) 
                  (letrec ((keyword (lambda (var ...) exp . rest))) (keyword init ...)))))

      ; library syntax:  (let* <bindings> <body>)
      (define-syntax let*
         (syntax-rules (<=)
            ((let* (((var ...) gen) . rest) . body)
               (receive gen (lambda (var ...) (let* rest . body))))
            ((let* ((var val) . rest-bindings) exp . rest-exps)
               ((lambda (var) (let* rest-bindings exp . rest-exps)) val))
            ((let* ((var ... (op . args)) . rest-bindings) exp . rest-exps)
               (receive (op . args)
                  (lambda (var ...) 
                     (let* rest-bindings exp . rest-exps))))
            ((let* ((var ... node) . rest-bindings) exp . rest-exps)
               (bind node
                  (lambda (var ...) 
                     (let* rest-bindings exp . rest-exps))))
            ((let* (((name ...) <= value) . rest) . code)
               (bind value
                  (lambda (name ...)
                     (let* rest . code))))
            ((let* ()) exp)
            ((let* () exp . rest)
               ((lambda () exp . rest)))))

      ; lets === let*, TEMP!
      (define-syntax lets
         (syntax-rules ()
            ((lets . stuff) (let* . stuff))))


      ; -----------------
      ; 4.2.3  Sequencing

      ; library syntax:  (begin <expression1> <expression2> ...)
      (define-syntax begin
         (syntax-rules (define define-syntax letrec define-values let*-values) ; ===>
            ;((begin
            ;   (define-syntax key1 rules1)
            ;   (define-syntax key2 rules2) ... . rest)
            ;   (letrec-syntax ((key1 rules1) (key2 rules2) ...)
            ;      (begin . rest)))
            ((begin exp) exp)
            ;((begin expression ===> wanted . rest)  ;; inlined assertions
            ;   (begin
            ;   (let ((val expression))
            ;      (if (eq? val (quote wanted)) #t
            ;         (sys '() 5 "assertion error: " (cons (quote expression) (cons "must be" (cons wanted '()))))))
            ;   (begin . rest)))
            ((begin (define . a) (define . b) ... . rest)
               (begin 42 () (define . a) (define . b) ... . rest))
            ((begin (define-values (val ...) . body) . rest)
               (let*-values (((val ...) (begin . body))) . rest))
            ((begin 42 done (define ((op . args1) . args2) . body) . rest)
               (begin 42 done (define (op . args1) (lambda args2 . body)) . rest))
            ((begin 42 done (define (var . args) . body) . rest)
               (begin 42 done (define var (lambda args . body)) . rest))
            ((begin 42 done (define var exp1 exp2 . expn) . rest)
               (begin 42 done (define var (begin exp1 exp2 . expn)) . rest))
            ((begin 42 done (define var val) . rest)
               (begin 42 ((var val) . done) . rest))
            ((begin 42 done . exps)
               (begin 43 done () exps))
            ((begin 43 (a . b) c exps)
               (begin 43 b (a . c) exps))
            ((begin 43 () bindings exps)
               (letrec bindings (begin . exps)))
            ((begin first . rest)  
               ((lambda (free)
                  (begin . rest))
                  first))))


      ; -------------------
      ; 4.1.5  Conditionals
      ; Temporary hack: if inlines some predicates.
      (define-syntax if
         (syntax-rules 
            (not eq? and null? pair? empty? type =)
            ((if test exp) (if test exp #false))
            ((if (not test) then else) (if test else then))
            ((if (null? test) then else) (if (eq? test '()) then else))
            ((if (empty? test) then else) (if (eq? test #empty) then else)) ;; FIXME - handle with partial eval later
            ((if (eq? a b) then else) (_branch 0 a b then else))            
            ((if (a . b) then else) (let ((x (a . b))) (if x then else)))   ; or ((lambda (x) (if x then else)) (a . b))
            ((if #false then else) else)
            ((if #true then else) then)
            ((if test then else) (_branch 0 test #false else then))))

      ; ------------------
      ; 4.1.6  Assignments

      ;; 4.2  Derived expression types
      ; The constructs in this section are hygienic, as discussed in section 4.3. For reference purposes,
      ; section 7.3 gives macro definitions that will convert most of the constructs described in this
      ; section into the primitive constructs described in the previous section.

      ; -------------------
      ; 4.2.1  Conditionals

      ; library syntax:  (or <test1> ...)
      (define-syntax or
         (syntax-rules ()
            ((or) #false)
            ((or (a . b) . c)
               (let ((x (a . b)))
                  (or x . c)))
            ((or a . b)
               (if a a (or . b)))))

      ; library syntax:  (and <test1> ...)
      (define-syntax and
         (syntax-rules ()
            ((and) #true)
            ((and a) a)
            ((and a . b)
               (if a (and . b) #false))))
      
      ; library syntax:  (cond <clause1> <clause2> ...)
      (define-syntax cond
         (syntax-rules (else =>)
            ((cond) #false)
            ((cond (else exp . rest))
               (begin exp . rest))
            ((cond (clause => exp) . rest) 
               (let ((fresh clause))
                  (if fresh
                     (exp fresh)
                     (cond . rest))))
            ((cond (clause exp . rest-exps) . rest) 
               (if clause
                  (begin exp . rest-exps)
                  (cond . rest)))))

      ; library syntax:  (case <key> <clause1> <clause2> ...)
      (define-syntax case
         (syntax-rules (else eqv? memv =>)
            ((case (op . args) . clauses)
               (let ((fresh (op . args)))
                  (case fresh . clauses)))
            ((case thing) #false)
            ((case thing ((a) => exp) . clauses)
               (if (eqv? thing (quote a))
                  (exp thing)
                  (case thing . clauses)))
            ((case thing ((a ...) => exp) . clauses)
               (if (memv thing (quote (a ...)))
                  (exp thing)
                  (case thing . clauses)))
            ((case thing ((a) . body) . clauses)
               (if (eqv? thing (quote a))
                  (begin . body)
                  (case thing . clauses)))
            ((case thing (else => func))
               (func thing))
            ((case thing (else . body))
               (begin . body))
            ((case thing ((a . b) . body) . clauses)
               (if (memv thing (quote (a . b)))
                  (begin . body)
                  (case thing . clauses)))
            ((case thing (atom . then) . clauses) ;; added for (case (type foo) (type-foo thenfoo) (type-bar thenbar) ...)
               (if (eq? thing atom)
                  (begin . then)
                  (case thing . clauses)))))

      ; expand case-lambda syntax to to (_case-lambda <lambda> (_case-lambda ... (_case-lambda <lambda> <lambda)))
      (define-syntax case-lambda
         (syntax-rules (lambda _case-lambda)
            ((case-lambda) #false) 
            ; ^ should use syntax-error instead, but not yet sure if this will be used before error is defined
            ((case-lambda (formals . body))
               ;; downgrade to a run-of-the-mill lambda
               (lambda formals . body))
            ((case-lambda (formals . body) . rest)
               ;; make a list of options to be compiled to a chain of code bodies w/ jumps
               ;; note, could also merge to a jump table + sequence of codes, but it doesn't really matter
               ;; because speed-sensitive stuff will be compiled to C where this won't matter
               (_case-lambda (lambda formals . body)
                  (case-lambda . rest)))))


      ; ----------------
      ; 4.2.4  Iteration
      
      (define-syntax do
        (syntax-rules ()
          ((do 
            ((var init step) ...)
            (test expr ...)
            command ...)
           (let loop ((var init) ...)
            (if test 
               (begin expr ...)
               (loop step ...))))))

      ; -------------------------
      ; 4.2.5  Delayed evaluation
      ; 4.2.6  Quasiquotation
      (define-syntax quasiquote
         (syntax-rules (unquote quote unquote-splicing append _work _sharp_vector list->vector)
                                                   ;          ^         ^
                                                   ;          '-- mine  '-- added by the parser for #(... (a . b) ...) -> (_sharp_vector ... )
            ((quasiquote _work () (unquote exp)) exp)
            ((quasiquote _work (a . b) (unquote exp))
               (list 'unquote (quasiquote _work b exp)))
            ((quasiquote _work d (quasiquote . e))
               (list 'quasiquote
                  (quasiquote _work (() . d) . e)))
            ((quasiquote _work () ((unquote-splicing exp) . tl))
               (append exp
                  (quasiquote _work () tl)))
            ((quasiquote _work () (_sharp_vector . es))
               (list->vector
                  (quasiquote _work () es)))
            ((quasiquote _work d (a . b))  
               (cons (quasiquote _work d a) 
                     (quasiquote _work d b)))
            ((quasiquote _work d atom)
               (quote atom))
            ((quasiquote . stuff)
               (quasiquote _work () . stuff))))
      
      
      ;; 4.3  Macros
      ; 4.3.1  Binding constructs for syntactic keywords
      ; 4.3.2  Pattern language
      
      
      ;;; Chapter 5
      ;;; Program structure
      ;; 5.1  Programs
      ;; 5.2  Definitions
      (define-syntax define
         (syntax-rules (lambda) ;λ
            ((define op a b . c)
               (define op (begin a b . c)))
            ((define ((op . args) . more) . body)
               (define (op . args) (lambda more . body)))
            ((define (op . args) body)
               (define op
                  (letrec ((op (lambda args body))) op)))
            ((define name (lambda (var ...) . body))
               (_define name (rlambda (name) ((lambda (var ...) . body)) name)))
;            ((define name (λ (var ...) . body))
;               (_define name (rlambda (name) ((lambda (var ...) . body)) name)))
            ((define op val)
               (_define op val))))
               
;      ;; not defining directly because rlambda doesn't yet do variable arity
;      ;(define list ((lambda (x) x) (lambda x x)))
;
;      ;; fixme, should use a print-limited variant for debugging
;
       ; EXTENSION, unused!
;      (define-syntax define*
;         (syntax-rules (print list)
;            ((define* (op . args) . body)
;               (define (op . args) 
;                  (print " * " (list (quote op) . args))
;                  .  body))
;            ((define* name (lambda (arg ...) . body))
;               (define* (name arg ...) . body))))

      ; EXTENSION, maybe unused!
      ; the internal one is handled by begin. this is just for toplevel.
      (define-syntax define-values
         (syntax-rules (list)
            ((define-values (val ...) . body)
               (_define (val ...)
                  (let* ((val ... (begin . body)))
                     (list val ...))))))

      ; EXTENSION, maybe r7rs!
      (define-syntax let*-values
         (syntax-rules ()
            ((let*-values (((var ...) gen) . rest) . body)
               (receive gen
                  (lambda (var ...) (let*-values rest . body))))
            ((let*-values () . rest)
               (begin . rest))))
               
               
               
      ; 5.2.1  Top level definitions
      ; 5.2.2  Internal definitions
      ; 5.3  Syntax definitions
      
      
      ;;; Chapter 6
      ;;; Standard procedures
      ;
      ; This chapter describes Scheme's built-in procedures. The initial (or ``top level'') Scheme
      ; environment starts out with a number of variables bound to locations containing useful values,
      ; most of which are primitive procedures that manipulate data. For example, the variable abs is
      ; bound to (a location initially containing) a procedure of one argument that computes the
      ; absolute value of a number, and the variable + is bound to a procedure that computes sums.
      ; Built-in procedures that can easily be written in terms of other built-in procedures are
      ; identified as ``library procedures''.
      ;
      ;
      ;; ....................
      
      ;; 6.1  Equivalence predicates
      
      ; procedure:  (eqv? obj1 obj2) 
      ; tbd.
      
      ; procedure:  (eq? obj1 obj2)    * builtin
      
      ;; 6.2  Numbers
      ; This data types related to olvm
      ;     - not a part of r5rs -     
      (define type-fix+              0)
      (define type-fix-             32)
      (define type-int+             40)
      (define type-int-             41)
      (define type-rational         42)
      (define type-complex          43) ;; 3 free below
      
      ; 6.2.1  Numerical types
      ; 6.2.2  Exactness
      ; 6.2.3  Implementation restrictions
      ; 6.2.4  Syntax of numerical constants
      
      ; ---------------------------
      ; 6.2.5  Numerical operations
      
      ; procedure:  (number? obj) 
      (define (number? o)
         (case (type o)
            (type-fix+ #true)
            (type-fix- #true)
            (type-int+ #true)
            (type-int- #true)
            (type-rational #true)
            (type-complex #true)
            (else #false)))
      
      ; procedure:  (complex? obj) 
      ; procedure:  (real? obj) 
      ; procedure:  (rational? obj) 
      ; procedure:  (integer? obj) 
      ; procedure:  (exact? z) 
      ; procedure:  (inexact? z)       
      ; procedure:  (= z1 z2 z3 ...) 
      ; procedure:  (< x1 x2 x3 ...) 
      ; procedure:  (> x1 x2 x3 ...) 
      ; procedure:  (<= x1 x2 x3 ...) 
      ; procedure:  (>= x1 x2 x3 ...) 
      ; library procedure:  (zero? z) 
      ; library procedure:  (positive? x) 
      ; library procedure:  (negative? x) 
      ; library procedure:  (odd? n) 
      ; library procedure:  (even? n)       
      ; library procedure:  (max x1 x2 ...) 
      ; library procedure:  (min x1 x2 ...) 
      ; procedure:  (+ z1 ...) 
      ; procedure:  (* z1 ...) 
      ; procedure:  (- z1 z2) 
      ; procedure:  (- z) 
      ; optional procedure:  (- z1 z2 ...) 
      ; procedure:  (/ z1 z2) 
      ; procedure:  (/ z) 
      ; optional procedure:  (/ z1 z2 ...)       
      ; ........
      
      ; 6.2.6  Numerical input and output
      ; procedure:  (number->string z) 
      ; procedure:  (number->string z radix) 
      ; procedure:  (string->number string) 
      ; procedure:  (string->number string radix) 
      
      
      ;; *********************
      ;; 6.3  Other data types
      ;
      ; This section describes operations on some of Scheme's non-numeric data types: booleans, pairs,
      ; lists, symbols, characters, strings and vectors.
      ; This data types related to olvm
      ;     - not a part of r5rs -     
      (define type-pair              1)
      
      (define type-bytecode         16)
      (define type-proc             17)
      (define type-clos             18)
      (define type-vector-dispatch  15)
      (define type-vector-leaf      11)
      (define type-vector-raw       19) ;; see also TBVEC in c/ovm.c
      (define type-ff-black-leaf     8)
      (define type-symbol            4)
      (define type-tuple             2)
      (define type-symbol            4)
      (define type-rlist-node       14)
      (define type-rlist-spine      10)
      (define type-string            3)
      (define type-string-wide      22)
      (define type-string-dispatch  21)
      (define type-thread-state     31)
      (define type-record            5)

      ;; transitional trees or future ffs
      (define type-ff               24)
      (define type-ff-r             25)
      (define type-ff-red           26)
      (define type-ff-red-r         27)

      ; + type-ff-red, type-ff-right

      ; 8 - black ff leaf
      ;; IMMEDIATE
      
      (define type-eof              20) ;; moved from 4, clashing with symbols
;      (define type-const            13) ;; old type-null, moved from 1, clashing with pairs
      (define type-port             12)
      (define type-tcp-client       62)
      

      ; ---------------
      ; 6.3.1  Booleans

      ; library procedure:  (not obj) 
      (define (not x)
         (if x #false #true))

      ; library procedure:  (boolean? obj) 
      (define (boolean? o)
         (cond
            ((eq? o #true) #true)
            ((eq? o #false) #true)
            (else #false)))


      ; ----------------------
      ; 6.3.2. Pairs and lists

      ; procedure:  (pair? obj)
      (define (pair? o)
         (eq? (type o) type-pair))

      ; procedure:  (cons obj1 obj2)    * builtin
      ; procedure:  (car pair)          * builtin
      ; procedure:  (cdr pair)          * builtin
      ; procedure:  (set-car! pair obj) * builtin (not implemented yet)
      ; procedure:  (set-cdr! pair obj) * builtin (not implemented yet)
      ; library procedure:  (caar pair)
      ; library procedure:  (cadr pair)
      ; ...
      ; library procedure:  (cdddar pair)
      ; library procedure:  (cddddr pair)
      
      ; library procedure:  (null? obj)
      (define (null? x)
         (eq? x '()))
         
      ; library procedure:  (list? obj)
      (define (list? l)
         (cond
            ((null? l) #true)
            ((pair? l) (list? (cdr l)))
            (else #false)))

      ; library procedure:  (list obj ...)
      (define-syntax list
         (syntax-rules ()
            ((list) '())
            ((list a . b)
               (cons a (list . b)))))
               
      ; library procedure:  (length list)
      ; library procedure:  (append list ...)
      ; library procedure:  (reverse list)
      ; library procedure:  (list-tail list k)
      ; library procedure:  (list-ref list k)
      
      ; library procedure:  (memq obj list) 
      ; library procedure:  (memv obj list) 
      ; library procedure:  (member obj list) 
      
      ; library procedure:  (assq obj alist) 
      ; library procedure:  (assv obj alist) 
      ; library procedure:  (assoc obj alist) 
      

      ; --------------
      ; 6.3.3. Symbols
      
      ; procedure:  (symbol? obj)
      (define (symbol? o)
         (eq? (type o) type-symbol))
         
      ; procedure:  (symbol->string symbol) *tbd
      ; procedure:  (string->symbol string) *tbd


      ; 6.3.4. Characters
      ; (char? obj) procedure
      (define (char? o) (number? o))


      ; 6.3.5. Strings
      ;; (string? obj) procedure
      (define (string? o)
         (case (type o)
            (type-string #true)
            (type-string-wide #true)
            (type-string-dispatch #true)
            (else #false)))


      ; 6.3.6. Vectors
      ;; (vector? obj) procedure
      (define (vector? o) ; == raw or a variant of major type 11?
         (case (type o)
            (type-vector-raw #true)
            (type-vector-leaf #true)
            (type-vector-dispatch #true)
            (else #false)))


       ;; *********************
       ;; 6.4  Control features

       ; *ol* extension
       (define (ff? o)        ; OL extension
          (or (eq? o #empty)
              (eq? 24 (fxband (type o) #b1111100))))

       ; *ol* extension
       (define (bytecode? o)  ; OL extension
          (eq? (type o) type-bytecode))

       ; *ol* extension
       (define (function? o)  ; OL extension
          (case (type o)
             (type-proc #true)
             (type-clos #true)
             (type-bytecode #true)
             (else #false)))

       ; procedure:  (procedure? obj)
       (define (procedure? o)
          (or (function? o) (ff? o)))


       ; procedure:  (apply proc arg1 ... args)  *builtin

       ; library procedure:  (map proc list1 list2 ...)
       (define (map fn lst)
          (if (null? lst)
             '()
             (let*
                ((head tail lst)
                 (head (fn head))) ;; compute head first
                (cons head (map fn tail)))))

       ; experimental syntax for map for variable count of arguments
       ;  can be changed to map used (apply f (map car .)) and (map cdr .))
       ; todo: test and change map to this version
       (define map2 (case-lambda
          ((f a b c) (let loop ((a a)(b b)(c c))
                        (if (null? a)
                           '()
                           (cons (f (car a) (car b) (car c)) (loop (cdr a) (cdr b) (cdr c))))))
          ((f a b) (let loop ((a a)(b b))
                        (if (null? a)
                           '()
                           (cons (f (car a) (car b)) (loop (cdr a) (cdr b))))))
          ((f a) (let loop ((a a))
                        (if (null? a)
                           '()
                           (cons (f (car a)) (loop (cdr a))))))
          (() #f)))

       ; library procedure:  (for-each proc list1 list2 ...)
       ; library procedure:  (force promise)
       
       ; procedure:  (call-with-current-continuation proc)
       ; Continuation - http://en.wikipedia.org/wiki/Continuation
       (define apply-cont (raw type-bytecode '(#x54)))  ;; не экспортим, внутренняя

       (define call-with-current-continuation
          ('_sans_cps
             (λ (k f)
                (f k (case-lambda
                        ((c a) (k a))
                        ((c a b) (k a b))
                        ((c . x) (apply-cont k x))))))) ; (apply-cont k x)

       (define call/cc call-with-current-continuation)
       
       
       ; procedure:  (values obj ...)
       ; procedure:  (call-with-values producer consumer)
       ; procedure:  (dynamic-wind before thunk after)
       ; 
       
       ;; 6.5  Eval
       ; ...


       ;; **********
       ;; *OL Tuples

      (define-syntax tuple
         (syntax-rules ()
            ((tuple a . bs) ;; there are no such things as 0-tuples
               (mkt 2 a . bs))))

      ; replace this with typed destructuring compare later on 

      (define-syntax tuple-case
         (syntax-rules (else _ is eq? bind div)
            ((tuple-case (op . args) . rest)
               (let ((foo (op . args)))
                  (tuple-case foo . rest)))
            ;;; bind if the first value (literal) matches first of pattern
            ((tuple-case 42 tuple type ((this . vars) . body) . others)
               (if (eq? type (quote this))
                  (bind tuple
                     (lambda (ignore . vars) . body))
                  (tuple-case 42 tuple type . others)))
            ;;; bind to anything
            ((tuple-case 42 tuple type ((_ . vars) . body) . rest)
               (bind tuple
                  (lambda (ignore . vars) . body)))
            ;;; an else case needing the tuple
            ((tuple-case 42 tuple type (else is name . body))
               (let ((name tuple))
                  (begin . body)))
            ;;; a normal else clause
            ((tuple-case 42 tuple type (else . body))
               (begin . body))
            ;;; throw an error if nothing matches
            ((tuple-case 42 tuple type)
               (syntax-error "weird tuple-case"))
            ;;; get type and start walking
            ((tuple-case tuple case ...)
               (let ((type (ref tuple 1)))
                  (tuple-case 42 tuple type case ...)))))
       

;      (define-syntax assert
;         (syntax-rules (if sys eq?)
;            ((assert result expression . stuff)
;               (if (eq? expression result) #t
;                  (sys '() 5 "assertion error: " (cons (quote expression) (cons "must be" (cons result '()))))))))
;;                 (call/cc (λ (resume) (sys resume 5 "Assertion error: " (list (quote expression) (quote stuff)))))


      ;; note, no let-values yet, so using let*-values in define-values
; .......................
; .......................
; .......................
; .......................













      ; 4.1.1  Variable references


      ; i hate special characters, especially in such common operations.
      ; let* (let sequence) is way prettier and a bit more descriptive 


      ;; now a function

      ; 4.2.6  Quasiquotation


      (define-syntax ilist
         (syntax-rules ()
            ((ilist a) a)
            ((ilist a . b)
               (cons a (ilist . b)))))


      (define-syntax call-with-values
         (syntax-rules ()
            ((call-with-values (lambda () exp) (lambda (arg ...) body))
               (receive exp (lambda (arg ...) body)))
            ((call-with-values thunk (lambda (arg ...) body))
               (receive (thunk) (lambda (arg ...) body)))))






      (define-syntax define-library
         (syntax-rules (export import begin _define-library define-library)
            ;; push export to the end (should syntax-error on multiple exports before this)
            ((define-library x ... (export . e) term . tl)
             (define-library x ... term (export . e) . tl))

            ;; lift all imports above begins
            ;((define-library x ... (begin . b) (import-old . i) . tl)
            ; (define-library x ... (import-old . i) (begin . b) . tl))

            ;; convert to special form understood by the repl
            ;((define-library name (import-old . i) ... (begin . b) ... (export . e))
            ; (_define-library 'name '(import-old . i) ... '(begin . b) ... '(export . e)))

            ;; accept otherwise in whatever order
            ((define-library thing ...)
             (_define-library (quote thing) ...))

            ;; fail otherwise
            ((_ . wtf)
               (syntax-error "Weird library contents: " (quote . (define-library . wtf))))))

      ;; toplevel library operations expand to quoted values to be handled by the repl
      ;(define-syntax import  (syntax-rules (_import)  ((import  thing ...) (_import  (quote thing) ...))))
      ;(define-syntax include (syntax-rules (_include) ((include thing ...) (_include (quote thing) ...))))


;      (define o (λ (f g) (λ (x) (f (g x)))))
;
;      (define i (λ (x) x))
;
;      (define self i)
;
;      ; (define call/cc  ('_sans_cps (λ (k f) (f k (λ (r a) (k a))))))
;
;      (define (i x) x)
;      (define (k x y) x)


      ;; these are core data structure type tags which are fixed and some also relied on by the vm

      ;; ALLOCATED


      ;;           allocated/pointers     allocated/rawdata    immediate
      ;; (size  x)         n                       n               #false
      ;; (sizeb x)       #false                    n               #false

      (define (immediate? obj) (eq? #false (size obj)))
      (define allocated? size)
      (define raw?       sizeb)






      ; 3.2. Disjointness of types
      ; No object satisfies more than one of the following predicates:
      ;
      ; boolean?          pair?
      ; symbol?           number?
      ; char?             string?
      ; vector?           port?
      ; procedure?
      ;
      ; These predicates define the types boolean, pair, symbol, number, char (or character), string, vector, port, and procedure. The empty list is a special object of its own type; it satisfies none of the above predicates.
      ;
      ; Although there is a separate boolean type, any Scheme value can be used as a boolean value for the purpose of a conditional test. As explained in section 6.3.1, all values count as true in such a test except for #f. This report uses the word ``true'' to refer to any Scheme value except #f, and the word ``false'' to refer to #f. 
      (define (port? o)
         (eq? (type o) type-port))




      ; 4. Expressions
      ; 4.1 Primitive expression types
      ; 4.1.1 Variable references

      ; 4.1.1  Variable references
;      (define-syntax define
;         (syntax-rules (lambda λ)
;            ((define op a b . c)
;               (define op (begin a b . c)))
;            ((define ((op . args) . more) . body)
;               (define (op . args) (lambda more . body)))
;            ((define (op . args) body)
;               (define op
;                  (letrec ((op (lambda args body))) op)))
;            ((define name (lambda (var ...) . body))
;               (_define name (rlambda (name) ((lambda (var ...) . body)) name)))
;;            ((define name (λ (var ...) . body)) ; fasten for (λ) process
;;               (_define name (rlambda (name) ((lambda (var ...) . body)) name)))
;            ((define op val)
;               (_define op val))))

      ; 4.1.2 Literal expressions
      ; ...




      ; 6. Standard procedures


      ;; (complex? obj ) procedure
      ;; (real? obj ) procedure
      ;; (rational? obj ) procedure
      ;; (integer? obj ) procedure


      ; 6.3. Other data types
      ; 6.3.1. Booleans
      ;; (not obj) library procedure


      ; 
      ; 

      ; 6.4 Control features


      ;(assert #t (procedure? car))
      ;(assert #f (procedure? 'car))
      ;(assert #t (procedure? (lambda (x) x)))
      

      ;; essential procedure: apply proc args
      ;; procedure: apply proc arg1 ... args
      (define apply      (raw type-bytecode '(#x14)))  ;; <- no arity, just call 20

      ;; ...

      ;; procedure: call-with-current-continuation proc

      ; non standard, owl extension
      (define-syntax lets/cc
         (syntax-rules (call/cc)
            ((lets/cc (om . nom) . fail) 
               (syntax-error "let/cc: continuation name cannot be " (quote (om . nom)))) 
            ((lets/cc var . body) 
               (call/cc (λ (var) (lets . body))))))

)
; ---------------------------
   (export
      λ syntax-error ;assert

      begin 
      quasiquote letrec let if 
      letrec* let*-values
      cond case define ;define*
      lets let* or and list
      ilist tuple tuple-case 
      call-with-values do define-library
      case-lambda
      define-values
      not
      
      ; список типов
      type-complex
      type-rational
      type-int+
      type-int-
      type-record

      immediate? allocated? raw?

      type-bytecode
      type-proc
      type-clos
      type-fix+
      type-fix-
      type-pair
      type-vector-dispatch
      type-vector-leaf
      type-vector-raw
      type-ff-black-leaf
      type-eof
      type-tuple
      type-symbol
;      type-const
      type-rlist-spine
      type-rlist-node
      type-port 
      type-tcp-client ; todo: remove and use (cons 'tcp-client port)
      type-string
      type-string-wide
      type-string-dispatch
      type-thread-state

      ;; sketching types
      type-ff               ;; k v, k v l, k v l r, black node with children in order
      type-ff-r             ;; k v r, black node, only right black child
      type-ff-red           ;; k v, k v l, k v l r, red node with (black) children in order
      type-ff-red-r         ;; k v r, red node, only right (black) child

      ;; k v, l k v r       -- type-ff
      ;; k v r, k v l r+    -- type-ff-right
      ;; k v l, k v l+ r    -- type-ff-leftc


      apply
      call-with-current-continuation call/cc lets/cc

      ; 3.2.
      boolean? pair? symbol? number? char? string? vector? port? procedure? null?
      ; ol extension:
      bytecode? function? ff?
      
      map list? map2
   )

)