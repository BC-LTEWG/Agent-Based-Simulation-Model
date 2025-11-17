# BC-LTEWG
Code and supporting documents for the Labor-Time Economy Working Group project at Boston College.

## For Developers
### Git Workflow
1. Create a branch if (and only if) you do not already have one:
   `git pull origin main`
   `git checkout -b <new-branch-name>`
   Make your changes.
   `git commit -am "... Commit message ..."
   `git push -u origin <new-branch-name>`
2. Continue to develop locally on your branch.  Push every time you commit.
3. If anybody else merges into `main` on GitHub, update your copy of `main` and its reflection in your branch:
   `git switch main`
   `git pull`
   `git switch <your-branch>`
   `git merge main`
   Resolve any conflicts if necessary. Commit and push.
4. When your changes are ready for review, create a new pull request (PR) on GitHub in your branch.
   - Be sure to name your PR appropriately with a short summary description.
   - Make your longer description clear.  Use imperative verbs and describe concepts and purposes.
   - At the bottom of the description, write the keyword "Resolves" followed by a space and then the "#" sign and the number of the issue that your changes address.  There should always be an issue for every PR.
   - Choose at least one review according to the round-robin review scheme (see below).
5. Respond to reviewers' comments.
6. When your code is approved, merge it into main on GitHub.
7. Be sure to update your local copy of `main` as above: `git switch main`; `git pull`; `git switch <your-branch>`; `git merge main`.

### Reviews
Every time anybody requests a review, that author should request it from the next person in this list, in round-robin fashion. When requesting a review, please check the current pending PRs to see who was the last to receive a request, and send your request to the next person on the list.  If there are no pending PRs, please use the most recent closed PR for this purpose.

- Amittai Aviram
- Alex Creiner
- Devin Lim
- Sal Nuhin
- Zhejun Zhang

