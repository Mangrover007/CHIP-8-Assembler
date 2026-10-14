In order to execute my plan to write a Tetris ROM for an
emulator I wrote myself, I need to write an Assembler for
the emulator.

Otherwise, the entire process would be painfully slow.

-----------------------------------------------------------

I have this in mind (after getting inspired by this stackoverflow
answer: )

```
start:
    CLS
    LD V1, 0
    LD V2, 10

    LD I, 0x200
    
    DRW V1, V2, 5

end:
    JMP #end    ; infinite loop
```

As I have read from [this post](https://nnarain.github.io/2017/09/30/Chip8-Assembler-Introduction.html),
there are 3 columns in an assembly language.

[label] [instruction] [comment]

In my case, [label] and [comment] will be optional.

Syntax:
```
<[label]:> [instruction] <; [comment]>
```

[label]

Label will be anything that is not one of the ~30 reserved keywords
used for identifying instructions. It will start with a letter,
followed by alphanumeric characters and end with a colon `:`.


Now that I have implemented the Lexer, I now need to implement the parser.
But what the fuck will I do after doing that?

Say I have implemented the parser. The next step would be to somehow go from
an "Abstract Syntax Tree" (whatever that means) to literal binary. I guess for
that, I need to study about "Code Generation", which is the next step after
parsers.

