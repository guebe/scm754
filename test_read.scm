; (c) guenter.ebermann@htl-hl.ac.at
; R7RS scheme reader testcases

; 2.1 Identifiers
+
+soup+
<=?
->string
a34kTMNs
lambda
list->vector
q
V17a
the-word-recursion-has-many-meanings

; 2.2 Whitespace and comment
  a     	         a   	       ; same as next line 
  a              a; same as next line 
a a

; 3.3. External representation
28 ; same as next line
#x1c
( 08 13 )
(8 . (13 . ()))
(+ 2 6)

; 4.1.2 Literal expression
'a
'()
'(+ 1 2)
'(quote a) ; same as next line
''a
'145932
'"abc"
'#t

; 6.2 Numbers
100
-100
1e2
3.14159265358979
-3.14159265358979
#x100
#o100
#b100
#d100

; 6.3 Booleans
#t
#f
'#f

; 6.4 Pairs and lists
()
(())
((()))
(()())
(()()())
(a b c d e) ; same as next line
(a . (b . (c . (d . (e . ())))))
(a b c . d) ; same as next line
(a . (b . (c . d)))
(a . b)
(a . ()) ; same as next line
(a)
((a) . (b c d)) ; same as next line
((a) b c d)
("a" . (b c)) ; same as next line
("a" b c)
((a b) . c)
(a (b) (c d e))
((a 1) (b 2) (c 3))
(a (b (c (d (e)))))

; 6.5 Symbols
foo
a
nil
flying-fish
Martin
Malvina
+
-

; 6.6. Characters
#\a ; lower case letter
#\A ; upper case letter
#\( ; left parenthesis
#\ ; the space character

; 6.7 Strings
"Another example: one line of text"
"Here's text 
containing two lines"
