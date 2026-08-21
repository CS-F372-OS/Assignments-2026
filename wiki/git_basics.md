# GitHub Command-Line Mastery

Welcome to the world of Git! Forget GUIs for a moment and embrace the power of the command line. This guide will walk you through the essential commands to make you a Git wizard.

---

## 1. The Genesis: `git clone`

Every great journey begins with a single step. In Git, it starts with `clone`. This command copies an existing repository from GitHub to your local machine.

**How to use it:**

```bash
# Clones a repository into a new directory
git clone https://github.com/<repository-owner>/<repository-name>.git
```

You now have a complete local copy of the project, history and all.

---

## 2. The Workspace: `status`, `add`, and `commit`

This is your daily bread and butter. You've made some changes, and now you need to save them.

### `git status`

Checks the status of your working directory. It shows which files are modified, which are new (untracked), and which are staged.

**How to use it:**

```bash
# See what's changed since your last commit
git status
```

### `git add`

Adds your changes to the "staging area". Think of this as a waiting room for your commits. You can add specific files or all of them.

**How to use it:**

```bash
# Add a specific file
git add path/to/your/file

# Add all modified and new files in the current directory (Think twice before using this!)
git add .
```

### `git commit`

Saves your staged changes to your local repository's history. A commit is a snapshot of your code at a specific point in time. Always write a clear, concise commit message!

**How to use it:**

```bash
# Commit your staged changes with a message
git commit -m "feat: Implement the new login feature"
```

---

## 3. Collaboration: `push` and `pull`

You've saved your work locally. Now it's time to share it with your team or sync up with their changes.

### `git push`

Uploads your committed changes from your local repository to the GitHub repository.

**How to use it:**

```bash
# Push your changes to the 'main' branch on the 'origin' remote
git push origin main
```

### `git pull`

Fetches changes from the remote repository and merges them into your current local branch. It's how you get the latest updates from others.

**How to use it:**

```bash
# Pull the latest changes from the 'main' branch
git pull origin main
```

---

## 4. Parallel Universes: `branch` and `checkout`

Branches let you work on new features or bug fixes in an isolated environment without affecting the main codebase.

### `git branch`

Lists all branches in your repository. You can also use it to create or delete branches.

**How to use it:**

```bash
# List all branches
git branch

# Create a new branch called 'new-feature'
git branch new-feature
```

### `git checkout`

Switches between different branches.

**How to use it:**

```bash
# Switch to the 'new-feature' branch
git checkout new-feature

# You can also create and switch to a new branch in one command!
git checkout -b another-new-feature
```

---

## 5. The Convergence: `merge`

Once your feature is complete and tested in its branch, you can merge it back into the main branch.

### `git merge`

Combines the history of two branches. You first `checkout` the branch you want to merge *into* (e.g., `main`), and then run `merge` with the name of the branch you want to merge *from*.

**How to use it:**

```bash
# First, switch to the main branch
git checkout main

# Then, merge the 'new-feature' branch into main
git merge new-feature
```

---

## 6. The Time Machine: `log`

Want to see the history of your project? `git log` is your DeLorean.

### `git log`

Shows a log of all the commits in your repository's history.

**How to use it:**

```bash
# Show the full commit history
git log

# Show a more condensed version of the log
git log --oneline --graph --decorate
```

---

## 7. A Typical Team Workflow

So how does this all fit together in a team? Here's a common sequence of events for a developer starting a new task.

**Step 1: Get the latest code**

Before starting, make sure your local `main` branch is up-to-date with the remote repository.

```bash
# Switch to the main branch
git checkout main

# Pull the latest changes
git pull origin main
```

**Step 2: Create your own branch**

Create a new branch to work on your feature in isolation.

```bash
# Create and switch to a new branch for your feature
git checkout -b new-feature-sprint-5
```

**Step 3: Do your work (Code, code, code!)**

Make all your code changes for the new feature. Once you're ready, save your work.

```bash
# Check the status of your changes
git status

# Add your changed files to the staging area
git add .

# Commit your changes with a descriptive message
git commit -m "feat: Implement user authentication"
```

**Step 4: Push your branch to GitHub**

Share your new feature branch with the remote repository.

```bash
# Push your branch
git push origin new-feature-sprint-5
```

**Step 5: Create a Pull Request (PR)**

Go to the repository on GitHub. You'll see a prompt to create a **Pull Request** from your `new-feature-sprint-5` branch to the `main` branch. Open the PR, describe your changes, and request a review from your teammates.

Once your PR is approved and merged, your code is now part of the `main` branch! You can now go back to Step 1 for your next task.

---

Happy coding!
