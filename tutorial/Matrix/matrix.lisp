#!/usr/bin/ol
(import (lib opengl))
(import (OpenGL version-1-1)
      (OpenGL EXT bgra))

(define rand!
   (let* ((ss ms (clock))
          (seed (band (+ ss ms) #xffffffff))
          (seed (cons (band seed #xffffff) (>> seed 24))))
      (lambda (limit)
         (let*((next (+ (car seed) (<< (cdr seed) 24)))
               (next (+ (* next 1103515245) 12345)))
            (set-car! seed (band     next     #xffffff))
            (set-cdr! seed (band (>> next 24) #xffffff))

            (mod (mod (floor (/ next 65536)) 32768) limit)))))


(define WIDTH  (/ 64 1)) ;(floor (/ 640 GLYPH_WIDTH))) ; /14 = 45.7
(define HEIGHT (/ 56 1)) ;(floor (/ 480 GLYPH_HEIGHT))) ;/16 = 30

(define NGLYPHS 28)           ; constant
(define SWITCH_FADE  0)        ; Затемненный иероглиф
(define SWITCH_PLAIN 1)        ; Обычный иероглиф
(define SWITCH_GLOW  2)        ; Подсвеченный иероглиф
(define SLIDING-MODE 1)        ; 0 - без слайда, 1 - частичный слад, 2 - только слайд

(define PHOSPHOR-ENABLED #true)
(define RANDGLOW-ENABLED #true)

(define SLIDING-DISABLED #true)

(define config (list->ff (list
   (cons 'density  60)
   (cons 'glowrate 10))))


(define (ne? x y)
   (not (eq? x y)))

(define (ith vector i)
   (if (eq? i 0) vector
      (ith (cdr vector) (- i 1))))


(define (create-scalar value)
   (cons value null))
(define (create-vector size)
   (repeat 0 size))
(define (create-matrix size-x size-y)
   (let loop ((n size-y) (out null))
      (if (eq? n 0)
         out
         (loop (- n 1) (cons (repeat 0 size-x) out)))))

(define (get-value scalar)
   (car scalar))
(define (get-vector-value vector i)
   (car (ith vector i)))
(define (get-matrix-value matrix i j)
   (car (ith (car (ith matrix j)) i)))


(define set! (case-lambda
((matrix i j value)
   (set-car! (ith (car (ith matrix j)) i) value))
((vector i value)
   (set-car! (ith vector i) value))
((scalar value)
   (set-car! scalar value))))


; ===========================================================
;matrix():

(define cells (list->ff (list
   (cons 'glyph     (create-matrix WIDTH HEIGHT))
   (cons 'glow      (create-matrix WIDTH HEIGHT))
   (cons 'spinner   (create-matrix WIDTH HEIGHT))))) ; 1/0

(define feeders (list->ff (list
   (cons 'remaining (create-vector WIDTH))
   (cons 'throttle  (create-vector WIDTH))
   (cons 'y         (create-vector WIDTH)))))

(define spinners (list->ff (list
   (cons 'x (create-vector 101))
   (cons 'y (create-vector 101)))))

(define density (create-scalar (getf config 'density)))


; spinners:
(define (create_spinner i)
(let ((x (rand! WIDTH))
      (y (rand! HEIGHT)))
;   (print "create new spinner " i "(" x "," y ")")
   (set! (getf spinners 'x) i x)
   (set! (getf spinners 'y) i y)
   (set! (getf cells 'spinner) x y 1)))

(define (clear_spinner i)
(let ((x (get-vector-value (getf spinners 'x) i))
      (y (get-vector-value (getf spinners 'y) i)))
   (set! (getf cells 'spinner) x y  0)))


(define spinners_length (create-scalar 0))
(define spinners_new_length (create-scalar 20)) ; config.spinners, may change

(define (densitizer density)
   (cond
      ((< density 10) 85)
      ((< density 15) 60)
      ((< density 20) 45)
      ((< density 25) 25)
      ((< density 30) 20)
      ((< density 35) 15)
      ((< density 45) 10)
      ((< density 50)  8)
      ((< density 55)  7)
      ((< density 65)  5)
      ((< density 80)  3)
      ((< density 90)  2)
      (else 1)))
;(runtime-error "debug-exit" '())

;
(define (insert_glyph2 glyph x y slide)
;(if (< y HEIGHT)
(let ((bottom_feeder_p (>= y 0)))
(let ((y (if bottom_feeder_p
            y
            (begin
               (let loop ((y (- HEIGHT 1)))
                  (if (eq? y 0)
                     0
                     (begin
                        (if (and
                              PHOSPHOR-ENABLED
                              (ne? (get-matrix-value (getf cells 'glyph) x y) 0)
                              (eq? (get-matrix-value (getf cells 'glyph) x (- y 1)) 0))
                           (set! (getf cells 'glow) x y -1)
                           (begin
                              (set! (getf cells 'glow ) x y (get-matrix-value (getf cells 'glow ) x (- y 1)))
                              (set! (getf cells 'glyph) x y (get-matrix-value (getf cells 'glyph) x (- y 1)))))
                        ;(set! (getf cells 'changed ) x y 1)
                        (loop (- y 1)))))))))

   (set! (getf cells 'glyph) x y glyph)
   ;(set! (getf cells 'changed) x y 0)

   (if (eq? glyph 0)
      (if bottom_feeder_p
         (set! (getf cells 'glow ) x y (+ 1 (rand! 2)))
         (set! (getf cells 'glow ) x y 0))))))

(define (insert_glyph glyph x y)
(if (< y HEIGHT)
(let ((bottom_feeder_p (>= y 0)))
(let ((y (if bottom_feeder_p
            y
            (begin
               (let loop ((y (- HEIGHT 1)))
                  (if (eq? y 0)
                     0
                     (begin
                        (if (and
                              PHOSPHOR-ENABLED
                              (ne? (get-matrix-value (getf cells 'glyph) x y) 0)
                              (eq? (get-matrix-value (getf cells 'glyph) x (- y 1)) 0))
                           (set! (getf cells 'glow) x y -1)
                           (begin
                              (set! (getf cells 'glow ) x y (get-matrix-value (getf cells 'glow ) x (- y 1)))
                              (set! (getf cells 'glyph) x y (get-matrix-value (getf cells 'glyph) x (- y 1)))))
                        ;(set! (getf cells 'changed ) x y 1)
                        (loop (- y 1)))))))))

   (set! (getf cells 'glyph) x y glyph)
   ;(set! (getf cells 'changed) x y 0)

   (if (eq? glyph 0)
      (if bottom_feeder_p
         (set! (getf cells 'glow ) x y (+ 1 (rand! 2)))
         (set! (getf cells 'glow ) x y 0)))))))


(gl:run

   "2. Drawing simple triangle"

; init
(lambda ()
   (glShadeModel GL_SMOOTH)
   (glClearColor 0 0 0 1.0)

   (glEnable GL_TEXTURE_2D)
   (glBindTexture GL_TEXTURE_2D 0)
   (glTexParameteri GL_TEXTURE_2D GL_TEXTURE_MAG_FILTER GL_LINEAR)
   (glTexParameteri GL_TEXTURE_2D GL_TEXTURE_MIN_FILTER GL_LINEAR)
   (glTexImage2D GL_TEXTURE_2D 0 GL_RGB8
      42 448
      0 GL_RGB GL_UNSIGNED_BYTE (file->vector "matrix.rgb"))

; изменение состояния матрицы:
; matrix->setDensity(100.0f * (pi.CommitTotal / (float)pi.CommitLimit));
; matrix->setUsage(10);
; matrix->Update()

   (glMatrixMode GL_PROJECTION)
   (glLoadIdentity)
   (glScalef -1 -1 1)
   (glOrtho 0 WIDTH 0 HEIGHT 0 1)

   (glMatrixMode GL_MODELVIEW)
   (glLoadIdentity)

   (let* ((time _ (clock)))
      (list time))
)

; draw
(lambda (oldtime)
   (let* ((time _ (clock)))

   (if #t ;(not (= oldtime time))
   (begin
   ; feed matrix
   (if #t
   (let loop ((x 0))
      (if (< x WIDTH)
         (let ((throttle  (get-vector-value (getf feeders 'throttle)  x))
               (remaining (get-vector-value (getf feeders 'remaining) x))
               (y         (get-vector-value (getf feeders 'y)         x)))

            (cond
               ((> throttle 0)
                  (set! (getf feeders 'throttle) x (- throttle 1)))
               ((> remaining 0)
                  (insert_glyph (+ (rand! NGLYPHS) 1) x y)
                  (set! (getf feeders 'remaining) x (- remaining 1))
                  (if (>= y 0)
                     (set! (getf feeders 'y) x (+ y 1))))
               (else
                  (insert_glyph 0 x y)
                  (if (>= y 0)
                     (set! (getf feeders 'y) x (+ y 1)))))
            (if (eq? (rand! 10) 0)
               (set! (getf feeders 'throttle) x (+ (rand! 5) (rand! 5))))

            (loop (+ x 1)))))
   )

   ; hack matrix:

   ;; implemented glow rate here -- just an arbitary value to multiply by
   (if RANDGLOW-ENABLED
      (let loop ((i (rand! (floor (/ (* (getf config 'glowrate) (/ WIDTH 2)) 10)))))
         (if (> i 0)
            (let ((x (rand! WIDTH))
                  (y (rand! HEIGHT)))
               (if (and
                     (ne? (get-matrix-value (getf cells 'glyph) x y) 0)
                     (eq? (get-matrix-value (getf cells 'glow ) x y) 0))
                  (begin
                     (set! (getf cells 'glow ) x y (rand! 20))
                     )) ;(set! (getf cells 'changed) x y 1)
               (loop (- i 1))))))
   ;; Change some of the feeders
   (if #t
   (let loop ((x 0))
      (if (< x WIDTH) (begin
         (if (or
               (eq? (get-vector-value (getf feeders 'remaining) x) 0)
               (eq? (rand! (densitizer (get-value density)))       0))
            (begin
               (set! (getf feeders 'remaining) x (+ 3 (rand! HEIGHT)))
               (set! (getf feeders 'throttle ) x (+ (rand! 5) (rand! 5)))
               (if (> (rand! 4) 0)
                  (set! (getf feeders 'remaining) x 0))

               (case SLIDING-MODE
                  (0 (set! (getf feeders 'y) x  (rand! HEIGHT)))
                  (1 (set! (getf feeders 'y) x  (if (eq? (rand! 2) 0) -1 (rand! HEIGHT))))
                  (2 (set! (getf feeders 'y) x -1)))))
         (loop (+ x 1)))))
   )

   ; скорость обновления спиннеров - в 5 раз ниже матрицы
   ; спиннеры можно вынести в отдельный массив и рендерить поверх основной матрицы

   (if #t
   (if (eq? (rand! 50) 0) (begin
      ; update spinners
;      (print "spinners: " spinners_length ", " spinners_new_length)
      (cond
         ((> (get-value spinners_new_length) (get-value spinners_length))
;            (print "up")
            (set! spinners_length (+ (get-value spinners_length) 1))
            (create_spinner (get-value spinners_length)))
         ((< (get-value spinners_new_length) (get-value spinners_length))
;            (print "down")
            (set! spinners_length (- (get-value spinners_length) 1))
            (clear_spinner  (get-value spinners_length))))

      (if (ne? (get-value spinners_length) 0)
         (let ((i (rand! (get-value spinners_length))))
            (clear_spinner i)
            (create_spinner i)))))
   )

   ))

   ; renderer
   (glClear GL_COLOR_BUFFER_BIT)
   (glColor3f 1 1 1)
   (glBindTexture GL_TEXTURE_2D 0)

   (glBegin GL_QUADS)
   ; Let's draw the matrix!
   (let for-y ((y 0)
               (glow* (getf cells 'glow))
               (glyph* (getf cells 'glyph))
               (spinner* (getf cells 'spinner)))
      (if (< y HEIGHT) (begin
      (let for-x ((x 0)
                  (glow* (car glow*))
                  (glyph* (car glyph*))
                  (spinner* (car spinner*)))
         (if (< x WIDTH)
         (let ((glow (car glow*))
               (glyph (car glyph*))
               (spinner (car spinner*)))

         (let ((u (/ (cond
                     ((> spinner 0) SWITCH_GLOW)
                     ((> glow 0)    SWITCH_GLOW)
                     ((< glow 0)    SWITCH_FADE)
                     (else          SWITCH_PLAIN)) 3))
               (v (/ glyph NGLYPHS)))

            ;(print glyph ": " u ", " v)

            (glTexCoord2f    u         v)
            (glVertex2f x y)
            (glTexCoord2f    u      (+ v 1/28))
            (glVertex2f x (+ y 1))
            (glTexCoord2f (+ u 1/3) (+ v 1/28))
            (glVertex2f (+ x 1) (+ y 1))
            (glTexCoord2f (+ u 1/3)    v)
            (glVertex2f (+ x 1) y))

            ;cell->changed = 0;

            (if #t (begin ;(not (= oldtime time)) (begin
;
            (if (> glow 0)
               (set! (getf cells 'glow) x y (- glow 1)) ; cell->changed = 1;
            (if (< glow 0) (begin
               (set! (getf cells 'glow) x y (+ glow 1)) ; cell->changed = 1;
               (if (eq? glow -1)
                  (set! (getf cells 'glyph) x y  0))))) ; cell->changed = 1;

            (if (> spinner 0)
               (set! (getf cells 'glyph) x y (rand! NGLYPHS))))) ; cell->changed = 1;

            (for-x (+ x 1) (cdr glow*) (cdr glyph*) (cdr spinner*)))))
      (for-y (+ y 1) (cdr glow*) (cdr glyph*) (cdr spinner*)))))
   (glEnd)

   (list time))))