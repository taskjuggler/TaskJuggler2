<!DOCTYPE style-sheet PUBLIC
          "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY % html "IGNORE">
<![%html;[
<!ENTITY % print "IGNORE">
<!ENTITY docbook.dsl PUBLIC
         "-//Norman Walsh//DOCUMENT DocBook HTML Stylesheet//EN"
         CDATA dsssl>
]]>
<!ENTITY % print "INCLUDE">
<![%print;[
<!ENTITY docbook.dsl PUBLIC
         "-//Norman Walsh//DOCUMENT DocBook Print Stylesheet//EN"
         CDATA dsssl>
]]>
]>

<style-sheet>

<!--      PRINT       -->
<style-specification id="print" use="docbook">
<style-specification-body> 

;;; TeX backend can go to PS (where EPS is needed)
;;; or to PDF (where PNG is needed).  So, just
;;; omit the file extension altogether and let
;;; tex/pdfjadetex sort it out on its own.
(define (graphic-file filename)
 (let ((ext (file-extension filename)))
  (if (or (equal? 'backend 'tex) ;; Leave off the extension for TeX
          (not filename)
          (not %graphic-default-extension%)
          (member ext %graphic-extensions%))
      filename
      (string-append filename "." %graphic-default-extension%))))

;;; Full justification.
(define %default-quadding%
 'justify)

;;; To make URLs line wrap we use the TeX 'url' package.
;;; See also: jadetex.cfg
;; First we need to declare the 'formatting-instruction' flow class.
(declare-flow-object-class formatting-instruction
"UNREGISTERED::James Clark//Flow Object Class::formatting-instruction")
;; Then redefine ulink to use it.
(element ulink
  (make sequence
    (if (node-list-empty? (children (current-node)))
        ; ulink url="...", /ulink
        (make formatting-instruction
          data: (string-append "\\url{"
                               (attribute-string (normalize "url"))
                               "}"))
        (if (equal? (attribute-string (normalize "url"))
                    (data-of (current-node)))
        ; ulink url="http://...", http://..., /ulink
            (make formatting-instruction data:
                  (string-append "\\url{"
                                 (attribute-string (normalize "url"))
                                 "}"))
        ; ulink url="http://...", some text, /ulink
            (make sequence
              ($charseq$)
              (literal " (")
              (make formatting-instruction data:
                    (string-append "\\url{"
                                   (attribute-string (normalize "url"))
                                   "}"))
              (literal ")"))))))
;;; And redefine filename to use it too.
(element filename
  (make formatting-instruction
    data: (string-append "\\path{" (data-of (current-node)) "}")))

</style-specification-body>
</style-specification>

<!--      HTML       -->
<style-specification id="html" use="docbook">
<style-specification-body> 

(define %stylesheet%
 "taskjuggler.css")

(define %use-id-as-filename%
 #t)

(define %html-ext%
 ".html")

</style-specification-body>
</style-specification>

<external-specification id="docbook" document="docbook.dsl">

</style-sheet>

