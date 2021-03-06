;;;
;;; Value interning and conversions
;;;

; have mappings:
;  - strings->symbol
;  - bytes->bytecode (where returned bytecode may use spacial primops)
; (- intern-char     -> x → x')

(define-library (lang intern)
   (export
      bytes->symbol
      string->symbol
      symbol->string
      ;initialize-interner
      string->uninterned-symbol
      string->interned-symbol       ;; tree string → tree' symbol
      put-symbol                    ;; tree sym → tree'
      empty-symbol-tree
      intern-symbols
      ;defined?

      fork-intern-interner
      )

   (import
      (r5rs core)
      (owl string)
      (owl interop)
      (owl list)
      (owl math)
      (owl io)
      (owl ff)
      (owl tuple)
      (owl symbol))

   (begin
      ; hack warning, could use normal = and < here, but
      ; using primitives speeds up parsing a bit

      ; TODO: подумать о табличке символов как ff с хеш-ключами

      (define empty-symbol-tree #false)

      ; #false = s1 is less, 0 = equal, 1 = s1 is more
      (define (walk s1 s2)
         (cond
            ((null? s1)
               (cond
                  ((pair? s2) #false)
                  ((null? s2) 0)
                  (else (walk s1 (s2)))))
            ((pair? s1)
               (cond
                  ((pair? s2)
                     (lets
                        ((a as s1)
                         (b bs s2))
                        (cond
                           ((eq? a b) (walk as bs))
                           ((less? a b) #false)
                           (else #true))))
                  ((null? s2) 1)
                  (else (walk s1 (s2)))))
            (else (walk (s1) s2))))

      ; сравнить две строки
      (define (compare s1 s2)
         (walk (str-iter s1) (str-iter s2)))

      ; FIXME, add a typed ref instruction

      (define (string->uninterned-symbol str)
         (vm:new type-symbol str))

      (define (symbol->string ob)
         (ref ob 1))

      (define (string->symbol str)
         (interact 'intern str))


      ; lookup node str sym -> node' sym'

      (define (maybe-lookup-symbol node str)
         (if node
            (lets
               ((this (symbol->string (ref node 2)))
                (res (compare str this)))
               (cond
                  ((eq? res 0) ; match
                     (ref node 2))
                  (res
                     (maybe-lookup-symbol (ref node 1) str))
                  (else
                     (maybe-lookup-symbol (ref node 3) str))))
            #false))

      ;; node is an unbalanced trie of symbols (F | (Tuple L sym R))
      (define (put-symbol node sym)
         (if node
            (lets
               ((this (ref node 2))
                (res (compare (symbol->string sym) (symbol->string this))))
               (cond
                  ((eq? res 0)
                     (set-ref node 2 sym))
                  (res
                     (set-ref node 1
                        (put-symbol (ref node 1) sym)))
                  (else
                     (set-ref node 3
                        (put-symbol (ref node 3) sym)))))
            (tuple #false sym #false)))

      ;; note, only leaf strings for now
      (define (string->interned-symbol root str)
         (let ((old (maybe-lookup-symbol root str)))
            (if old
               (values root old)
               (let ((new (string->uninterned-symbol str)))
                  (values (put-symbol root new) new)))))

      ;;;
      ;;; BYTECODE INTERNING
      ;;;

      ; this will be forked as 'interner
      ; to bootstrap, collect all symbols from the entry procedure, intern
      ; them, and then make the intial threads with an up-to-date interner

      (define (bytes->symbol bytes)
         (string->symbol
            (runes->string
               (reverse bytes))))

      (define (intern-symbols sexp)
         (cond
            ((symbol? sexp)
               (string->symbol (ref sexp 1)))
            ((pair? sexp)
               (cons (intern-symbols (car sexp)) (intern-symbols (cdr sexp))))
            (else sexp)))

      ;; obj → bytecode | #false
      (define (bytecode-of func)
         (cond
            ((bytecode? func) func)
            ((function? func) (bytecode-of (ref func 1)))
            (else #false)))

      (define (debug . args)
         (apply print-to (cons stderr args))
         42)

      ;
      (define (fork-intern-interner symbols)
         (let ((root (fold put-symbol empty-symbol-tree symbols)))
            (fork-server 'intern (lambda ()
               (let loop ((root root))
                  (let*((envelope (wait-mail))
                        (sender msg envelope))
                     (cond
                        ((string? msg)
                           ;(debug "interner: interning bytecode")
                           (let*((root symbol (string->interned-symbol root msg)))
                              (mail sender symbol)
                              (loop root)))
                        ; todo: simplify this
                        (else
                           (mail sender 'bad-kitty)
                           (loop root)))))))))

      ; fixme: invalid
      ;(define-syntax defined?
      ;   (syntax-rules (*toplevel*)
      ;      ((defined? symbol)
      ;         (get *toplevel* symbol #false))))

))
