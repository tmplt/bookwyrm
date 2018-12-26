Contributing
---

> **Disclaimer**: this project is a result of work done in my spare time over the last three years.
> During this time, my programming patterns, proficiency with C++, among other factors have altered (hopefully for the better).
> As a result, some parts of this code base are messy, hard to understand, and generally a displeasure to look at.
> The biggest lesson learned here is that working on a bigger project without any plan at all is a very bad idea;
> multiple times have I considered the program useful, only to belatedly realise a change that would make everything better,
> effectively pushing said release forward multiple months.
>
> You are free to improve/extend any part of the code base, but if some component is too much spaghetti, it will eventually be rewritten.

You are welcome to contribute, be it code, bug reports, or feature requests.

Considering the still rather undefined direction of this project please contact me or start a discussion issue if you're planning some greater contribution which isn't already documented or doesn't have a related issue.

If you're looking for some smaller issues to fix, I've made sure to sprinkle `TODO: ...`s throughout the code base.
These comments denote pieces of code that should be improved, but don't deserve their own dedicated issues.
To find these comments, run the following:

```sh
$ cd $(git rev-parse --show-toplevel) # move to project root
$ grep -rn "TODO" --exclude-dir=lib .
```
