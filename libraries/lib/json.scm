(define-library (lib json)
   (export
      print-json-with)
   (import (otus lisp))

(begin

(define (print-json-with display object)
   (let jsonify ((L object))
      (cond
         ((symbol? L)
            (for-each display `("\"" ,L "\"")))
         ((string? L)
            (for-each display `("\"" ,L "\"")))
         ((boolean? L)
            (display (if L "true" "false")))

         ((integer? L)
            (display L))
         ((rational? L)
            (let*((int (floor L))
                  (frac (floor (* (- L int) 10000))))
            (display int) (display ".")
            (let loop ((i frac) (n 1000))
               (display (floor (/ i n)))
               (if (less? 1 n)
                  (loop (mod i n) (/ n 10))))))
         ((vector? L)
            (display "[")
            (let ((len (vec-len L)))
               (let loop ((n 0))
                  (if (less? n len) (begin
                     (jsonify (vector-ref L n))
                     (if (less? (+ n 1) len)
                        (display ","))
                     (loop (+ n 1))))))
            (display "]"))
         ((list? L)
            (display "{")
            (let loop ((L L))
               (if (not (null? L)) (begin
                  (for-each display `("\"" ,(caar L) "\":"))
                  (jsonify (cdar L))
                  (if (not (null? (cdr L)))
                     (display ","))
                  (loop (cdr L)))))
            (display "}")))))

))
