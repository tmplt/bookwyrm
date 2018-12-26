Contributing
---

You are welcome to contribute, be it code, bug reports, or feature requests.

Considering the still rather undefined direction of this project please contact me or start a discussion issue if you're planning some greater contribution which isn't already documented or doesn't have a related issue.

If you're looking for some smaller issues to fix, I've made sure to sprinkle `TODO: ...`s throughout the code base.
These comments denote pieces of code that should be improved, but don't deserve their own dedicated issues.
To find these comments, run the following:

```sh
$ cd $(git rev-parse --show-toplevel) # move to project root
$ grep -rn "TODO" --exclude-dir=lib .
```
