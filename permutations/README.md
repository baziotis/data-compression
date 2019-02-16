# Permutations

## Factorial number system
Permutations cannot be compressed simply (like simple delta encoding) because their elements are not ordered in any specific
way. That said, they have a very interesting property that we can exploit. <br/>
Permutations are sets of constant length. That is, each element appears exactly once, in every permutation of an initial set.
So, for the initial set 012, the possible permutations are: <br/>
012 021 102 120 201 210. <br/>
Given that property, we can bring some fancy maths to help us with the compression. One way to achieve smaller size
is to note that if we have a way to order permutations, then we don't need to save the permutation itself but instead we can
save which term in this order our permutation is. <br/>
If given the number of a term, we can find the permutation and given a permutation, we can find the term, then we're done.
The way to do that is using the factorial number system and you can find a good explanation in the source code. <br/>
<br/>
## Exclude one by one (because no other title made sense)
Another way to compress permutations is again by exploiting this property but now using a simpler technique.
Let's say that we want to compress the permutation: 0213 <br/>
We start by making an array of the digits that have not been used in the permutation (initially all): <br/>
Index: 0 | 1 | 2 | 3 <br/>
<br/>
We start by taking the first digit in the permutation, which is 0. Currently the array of digits has 4 items, so
we need to 2 bits (log2(4)) to save 0. The reason for that is that we think of the person decompressing that. No digit has
been used so we need a number of bits that will let him distinguish what is the starting digit among all the other digits.
So, we write:
00 <br/>
Then, we remove it from the array: <br/>
Index:  1 | 2 | 3 <br/>
<br/>
Then, we do the same thing for the next digit, which is 2. Again, we need 2 bits to distinguish it from the other 3, so
our compressed output is now: <br/>
00 10 <br/>
and the array: <br/>
Index:  1 | 3 <br/>
<br/>
We do the same thing for the next digit, which is 1. Notice that now we need only 1 bit to distinguish it from the 2 digits
remaining and that's exactly where we break the Shannon entropy. This is the point where we exploint the uniqueness
of each digit in the set by tracking which ones have been used. We only need 1 bit to distinguish 1 from 3
(the 2 digits remaining). So, now the output is: <br/>
00 10 0 <br/>
(Notice that we chose the 0 bit because we have 2 digits remaining, 1 and 3 and so it makes sense to assign the 0 bit to 1
and the 1 to 3. For bigger sets, that's the way of thinking as well). <br/>
<br/>
And finally for the last digit: <br/>
00 10 0 1 <br/>
<br/>
In that way, we achieved to compress the initial set from N * log(N) = 4 * log(4) = 4 * 2 = 8 bits to 6 bits. That's not a lot
because the set is small but it can be somewhat efficient for bigger sets.
