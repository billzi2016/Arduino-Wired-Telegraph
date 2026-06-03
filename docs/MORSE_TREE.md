# Morse Code Tree

## 1. Rule

This tree uses the standard binary traversal idea:

- Left branch = dot `.`
- Right branch = dash `-`

Start from the root and follow the path of dots and dashes until you reach the target character.

Example:

- `A = .-`
- Start at root
- Go left for `.`
- Then go right for `-`
- Result: `A`

---

## 2. Morse Tree For Letters And Digits

```text
ROOT
├── .  E
│   ├── .  I
│   │   ├── .  S
│   │   │   ├── .  H
│   │   │   │   ├── .  5
│   │   │   │   ├── -  4
│   │   │   ├── -  V
│   │   │       ├── .  ?
│   │   │       ├── -  3
│   │   ├── -  U
│   │       ├── .  F
│   │       │   ├── .  ?
│   │       │   ├── -  ?
│   │       ├── -  ?
│   │           ├── .  2
│   │           ├── -  ?
│   ├── -  A
│       ├── .  R
│       │   ├── .  L
│       │   │   ├── .  ?
│       │   │   ├── -  ?
│       │   ├── -  ?
│       │       ├── .  +
│       │       ├── -  ?
│       ├── -  W
│           ├── .  P
│           │   ├── .  ?
│           │   ├── -  ?
│           ├── -  J
│               ├── .  1
│               ├── -  ?
└── -  T
    ├── .  N
    │   ├── .  D
    │   │   ├── .  B
    │   │   │   ├── .  6
    │   │   │   ├── -  =
    │   │   ├── -  X
    │   │       ├── .  /
    │   │       ├── -  ?
    │   ├── -  K
    │       ├── .  C
    │       │   ├── .  ?
    │       │   ├── -  ?
    │       ├── -  Y
    │           ├── .  (
    │           ├── -  )
    ├── -  M
        ├── .  G
        │   ├── .  Z
        │   │   ├── .  7
        │   │   ├── -  ?
        │   ├── -  Q
        │       ├── .  ?
        │       ├── -  ?
        ├── -  O
            ├── .  Ö / Ø
            │   ├── .  8
            │   ├── -  ?
            ├── -  Ch
                ├── .  9
                ├── -  0
```

---

## 3. Compact Tree With Paths

This compact version is easier to use when implementing a decoder.

```text
""      -> ROOT
"."     -> E
"-"     -> T

".."    -> I
".-"    -> A
"-."    -> N
"--"    -> M

"..."   -> S
"..-"   -> U
".-."   -> R
".--"   -> W
"-.."   -> D
"-.-"   -> K
"--."   -> G
"---"   -> O

"...."  -> H
"...-"  -> V
"..-."  -> F
"..--"  -> ?
".-.."  -> L
".-.-"  -> ?
".--."  -> P
".---"  -> J
"-..."  -> B
"-..-"  -> X
"-.-."  -> C
"-.--"  -> Y
"--.."  -> Z
"--.-"  -> Q
"---."  -> Ö / Ø
"----"  -> Ch

"....." -> 5
"....-" -> 4
"...--" -> 3
"..---" -> 2
".----" -> 1
"-...." -> 6
"--..." -> 7
"---.." -> 8
"----." -> 9
"-----" -> 0
```

---

## 4. Recommended Tree Subset For This Project

For the first Arduino implementation, the recommended active decode tree is:

- Letters `A-Z`
- Digits `0-9`
- Space handled separately as a word gap

Recommended exclusions for the first version:

- Extended Latin characters
- Most punctuation
- Special procedural signals

This keeps the decoder simpler and reduces ambiguity during timing-based parsing.

---

## 5. How To Use This Tree In Code

Recommended decoder logic:

1. Collect one symbol at a time from timing input.
2. Convert short pulse to `.`
3. Convert long pulse to `-`
4. Append to a temporary Morse buffer
5. When a letter gap is detected, look up the complete path
6. Output the matching character
7. When a word gap is detected, output a space

Two practical implementation methods:

- Lookup table: use a string-to-character map
- Binary tree: traverse nodes directly as input arrives

For Arduino, a lookup table is simpler at first.  
A binary tree becomes useful if you want a streaming decoder with less string handling.

---

## 6. Minimal Decoder-Oriented Tree

If you want a cleaner implementation-only tree, this is the minimal subset:

```text
ROOT
├── .  E
│   ├── .  I
│   │   ├── .  S
│   │   │   ├── .  H
│   │   │   └── -  V
│   │   └── -  U
│   │       └── .  F
│   └── -  A
│       ├── .  R
│       │   └── .  L
│       └── -  W
│           ├── .  P
│           └── -  J
└── -  T
    ├── .  N
    │   ├── .  D
    │   │   ├── .  B
    │   │   └── -  X
    │   └── -  K
    │       ├── .  C
    │       └── -  Y
    └── -  M
        ├── .  G
        │   ├── .  Z
        │   └── -  Q
        └── -  O
```

This version is good for first-pass decoding of `A-Z`.

---
