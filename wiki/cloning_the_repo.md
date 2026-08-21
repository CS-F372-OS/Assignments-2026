# Cloning this Repository

To work with the project assignments while maintaining your own private repository, you'll need to create a private repository and mirror the public course repository to it.

## Prerequisites

Before you begin, ensure you have Git installed on your system:

- **Windows**: Download and install [Git for Windows](https://git-scm.com/download/win)
- **macOS**: Install via [Homebrew](https://brew.sh/) with `brew install git` or download from [git-scm.com](https://git-scm.com/download/mac)
- **Linux**: Install via your package manager:
  - Ubuntu/Debian: `sudo apt-get install git`
  - CentOS/RHEL: `sudo yum install git`
  - Fedora: `sudo dnf install git`

Verify Git is installed by running:
```bash
git --version
```

## Step 1: Create Your Private Repository

1. Go to [GitHub](https://github.com) and create a new repository under your account.
2. Name it `CSF372_OS_2026_GRP_<X>` where `X` is your group number Example - `CSF372_OS_2026_GRP_18`
3. Set the repository visibility to **Private**.
4. Skip this step if you already have created a private repo named `CSF372_OS_2026_GRP_<X>`.
5. Add `csf372.ta@gmail.com` as collaborator to your private repository.

## Step 2: Mirror the Public Repository

### Create a Bare Clone

On your development machine, create a bare clone of the public course repository:

```bash
git clone --bare https://github.com/CS-F372-OS/Assignments-2026.git assignments-public-temp
```

### Mirror to Your Private Repository

Navigate to the temporary directory and push to your private repository:

```bash
cd assignments-public-temp

# If you pull/push over HTTPS
git push https://github.com/<USERNAME>/CSF372_OS_2026_GRP_<X>.git main --force

# If you pull/push over SSH
git push git@github.com:<USERNAME>/CSF372_OS_2026_GRP_<X>.git main
```

Replace `<USERNAME>` with your GitHub username and `<X>` with your group number.

### Clean Up

Delete the temporary local clone of the public repository:

```bash
cd .. && rm -rf assignments-public-temp
```

## Step 3: Clone Your Private Repository

Clone your private repository to your development machine:

```bash
# If you pull/push over HTTPS
git clone https://github.com/<USERNAME>/CSF372_OS_2026_GRP_<X>.git

# If you pull/push over SSH
git clone git@github.com:<USERNAME>/CSF372_OS_2026_GRP_<X>.git
```

## Step 4: Add the Public Repository as Remote

Add the public course repository as a second remote to receive updates throughout the semester:

```bash
cd CSF372_OS_2026_GRP_<X>
git remote add public https://github.com/CS-F372-OS/Assignments-2026.git
```

Verify the remotes are configured correctly:

```bash
git remote -v
```

You should see output similar to:
```
origin  https://github.com/<USERNAME>/CSF372_OS_2026_GRP_<X>.git (fetch)
origin  https://github.com/<USERNAME>/CSF372_OS_2026_GRP_<X>.git (push)
public  https://github.com/CS-F372-OS/Assignments-2026.git (fetch)
public  https://github.com/CS-F372-OS/Assignments-2026.git (push)
```

## Step 5: Disable GitHub Actions (Important!)

To avoid running out of GitHub Actions quota:

1. Go to your private repository settings
2. Navigate to **Settings > Actions > General**
3. Under **Actions permissions**, select **Disable actions**

## Pulling Updates

Throughout the semester, you can pull in changes from the public course repository:

```bash
git pull public main
```

## Next Steps

Now that you have set up your repository, you can proceed with [Setting up Pintos](./pintos_manager.md).

