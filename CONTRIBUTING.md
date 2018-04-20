# Contributing

:+1::tada: First off, thanks for taking the time to contribute! :tada::+1:

Contributions are welcome and are greatly appreciated! Every little bit helps, and credit will always be given. 

The following is set of guidelines for contributing to all [Elastos repositories](https://github.com/elastos) hosted on Github. These are guidelines, not rules. So use your best judgement because some guidelines can differ due to used programming language.

#### Table Of Contents

[I don't want to read this whole thing, I just have a question!!!](#i-dont-want-to-read-this-whole-thing-i-just-have-a-question)

[How Can I Contribute?](#how-can-i-contribute)
  * [Report Bugs](#report-bugs)
  * [Suggesting Enhancements](#suggesting-enhancements)
  * [Your Code Contribution](#your-code-contribution)
  * [Pull Requests](#pull-requests)
  * [Commit Messages](#commit-messages)
  * [Example of Contribution](#example-of-contribution)

[Sources](#sources)

## I don't want to read this whole thing I just have a question!!!

> **Note:** Please don't file an issue to ask a question. You'll get faster results by using the resources below.
 
 * Read [Community FAQ](https://github.com/elastos/Elastos.Community#faq)
 * Join [Elastos Community Telegram](https://t.me/elastosgroup) for general questions
    * Even though Telegram is a chat service, sometimes it takes several hours for community members to respond &mdash; please be patient!
 * Join [Elastos Developers Telegram](https://t.me/elastosdev) for more technical questions
    * Core Developers are not usually watching this channel. If you need to contact core team, please use Github.
 * Create an issue with question in [Elastos](https://github.com/elastos/Elastos) for direct questions to core team.

## How Can I Contribute?

You can contribute in many ways:

### Report Bugs

This section guides you through submitting a bug. Following these guidelines helps maintainers and the community understand your report reproduce the behavior, and find related reports.

Before reporting bug, please perform basic troubleshooting steps:

* Make sure you’re on the latest version. If you’re not on the most recent version, your problem may have been solved already! Upgrading is always the best first step.
* Try older versions. If you’re already on the latest release, try rolling back a few minor versions (e.g. if on 1.7, try 1.5 or 1.6) and see if the problem goes away. This will help the devs narrow down when the problem first arose in the commit log.
* Try switching up dependency versions. If the software in question has dependencies (other libraries, etc) try upgrading/downgrading those as well.
* Determine [which repository the enhancement should be suggested in](https://github.com/elastos/Elastos)
* Search the repository issues to make sure it’s not a known issue.

If you don’t find a pre-existing bug, consider checking with the [Elastos Community Telegram](https://t.me/elastosgroup) channel in case the problem is non-bug-related.

When you are creating a bug report, please [include as many details as possible](#how-do-i-submit-bug-report). Fill out [the required template](ISSUE_TEMPLATE.md), the information it asks for helps us resolve issues faster.

> **Note:** If you find a **Closed** issue that seems like it is the same thing that you're experiencing, open a new issue and include a link to the original issue in the body of your new one.

#### How Do I Submit A Bug Report?

Bugs are tracked as [GitHub issues](https://guides.github.com/features/issues/). After you've determined [which repository](https://github.com/elastos) your bug is related to, create an issue on that repository and provide the following information by filling in [the template](ISSUE_TEMPLATE.md).

Explain the problem and include additional details to help maintainers reproduce the problem:

* **Use a clear and descriptive title** for the issue to identify the problem.
* **Describe the exact steps which reproduce the problem** in as many details as possible. For example, start by explaining how you make a build, e.g. which command exactly you used in the terminal, or if you used Docker.
* **Provide specific examples to demonstrate the steps**. Include links to files or GitHub projects, or copy/pasteable snippets, which you use in those examples. If you're providing snippets in the issue, use [Markdown code blocks](https://help.github.com/articles/markdown-basics/#multiple-lines).
* **Describe the behavior you observed after following the steps** and point out what exactly is the problem with that behavior.
* **Explain which behavior you expected to see instead and why.**
* **Include screenshots and animated GIFs** which show you following the described steps and clearly demonstrate the problem. You can use [this tool](https://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and [this tool](https://github.com/colinkeenan/silentcast) or [this tool](https://github.com/GNOME/byzanz) on Linux.
* **If you're reporting that program crashed**, include a crash report with a stack trace .
* **If the problem wasn't triggered by a specific action**, describe what you were doing before the problem happened and share more information using the guidelines below.

Include details about your configuration and environment:

* **Which version of repo are you using?** You can get the exact commit hash by running `git log -p -2` in your terminal.
* **What's the name and version of the OS you're using**?
* **Are you running app in a virtual machine or Docker?** If so, which VM software are you using and which operating systems and versions are used for the host and the guest?
* **Which required packages do you have installed?** 

### Suggesting Enhancements

This section guides you through submitting an enhancement. It includes new features and minor improvements to existing functionality. Following these guidelines helps maintainers and the community understand your suggestion and find related suggestions.

Before suggesting enhancement, please perform basic troubleshooting steps:

* Make sure you’re on the latest version. If you’re not on the most recent version, your enhancement may have been available already! Upgrading is always the best first step.
* Determine [which repository the enhancement should be suggested in](https://github.com/elastos/Elastos)
* Search the repository issues to see if the enhancement has already been suggested. If it has, add a comment to the existing issue instead of opening a new one.

#### How Do I Submit A Enhancement Suggestion?

Enhancements are tracked as [GitHub issues](https://guides.github.com/features/issues/). After you've determined [which repository](https://github.com/elastos) your enhancement is related to, create an issue on that repository and provide the following information:

* **Use a clear and descriptive title** for the issue to identify the suggestion.
* **Provide a step-by-step description of the suggested enhancement** in as many details as possible.
* **Provide specific examples to demonstrate the steps**. Include copy/pasteable snippets which you use in those examples, as [Markdown code blocks](https://help.github.com/articles/markdown-basics/#multiple-lines).
* **Describe the current behavior** and **explain which behavior you expected to see instead** and why.
* **Include screenshots and animated GIFs**. You can use [this tool](https://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and [this tool](https://github.com/colinkeenan/silentcast) or [this tool](https://github.com/GNOME/byzanz) on Linux.
* **Explain why this enhancement would be useful**
* **List some other text editors or applications where this enhancement exists.**
* **Specify which version of app you're using.** You can get the exact commit hash by running `git log -p -2` in your terminal.
* **Specify the name and version of the OS you're using.**

### Your Code Contribution

So you decided to contribute to some of [repository](https://github.com/elastos). Before contribution you should follow those rules:

* **Always make a new branch for your work, no matter how small.** This makes it easy for others to take just that one set of changes from your repository, in case you have multiple unrelated changes floating around.
* **Don’t submit unrelated changes in the same branch/pull request!** The maintainer shouldn’t have to reject your awesome bugfix because the feature you put in with it needs more review.
* **Bug fixes** should be based on the branch named after the oldest supported release line the bug affects.
    * E.g. if a feature was introduced in 1.1, the latest release line is 1.3, and a bug is found in that feature - make your branch based on 1.1. The maintainer will then forward-port it to 1.3 and master.
    * Bug fixes requiring large changes to the code or which have a chance of being otherwise disruptive, may need to base off of master instead. This is a judgement call – ask the devs!
* **New features** should branch off of the ‘master’ branch.
    * Note that depending on how long it takes for the dev team to merge your patch, the copy of master you worked off of may get out of date! If you find yourself ‘bumping’ a pull request that’s been sidelined for a while, make sure you rebase or merge to latest master to ensure a speedier resolution.
* **Follow the style you see used in the primary repository!** Consistency with the rest of the project always trumps other considerations. It doesn’t matter if you have your own style or if the rest of the code breaks with the greater community - just follow along.
* **Documentation isn’t optional**. Patches without documentation will be returned to sender. By “documentation” we mean:
    * **Docstrings** must be created or updated for public API functions/methods/etc.
    * New features should ideally include updates to prose documentation, including useful example code snippets.
    * All submissions should have a changelog entry crediting the contributor and/or any individuals instrumental in identifying the problem.
* **Tests aren’t optional**. Any bugfix that doesn’t include a test proving the existence of the bug being fixed, may be suspect. We’ve found that test-first development really helps make features better architected and identifies potential edge cases earlier instead of later. Writing tests before the implementation is strongly encouraged.

### Pull Requests

* Describe why are you doing this PR.
* Do not include issue numbers in the PR title.
* Include screenshots and animated GIFs in your pull request whenever possible.
* End all files with a newline.
* Please rebase or merge and resolve all conflicts before submitting.
* Please ensure the necessary checks pass and that code coverage does not decrease.
* If you are asked to update your pull request with some changes there's no need to create a new one. Push your changes to the same branch.

### Commit messages

* Use the present tense ("Add feature" not "Added feature")
* Use the imperative mood ("Move cursor to..." not "Moves cursor to...")
* Limit the first line to 50 characters or less
* Reference issues and pull requests liberally after the first line
* Consider starting the commit message with an applicable emoji:
    * :art: `:art:` when improving the format/structure of the code
    * :racehorse: `:racehorse:` when improving performance
    * :memo: `:memo:` when writing docs
    * :penguin: `:penguin:` when fixing something on Linux
    * :apple: `:apple:` when fixing something on macOS
    * :checkered_flag: `:checkered_flag:` when fixing something on Windows
    * :bug: `:bug:` when fixing a bug
    * :fire: `:fire:` when removing code or files
    * :white_check_mark: `:white_check_mark:` when adding tests
    * :arrow_up: `:arrow_up:` when upgrading dependencies
    * :arrow_down: `:arrow_down:` when downgrading dependencies
    * :shirt: `:shirt:` when removing linter warnings

Please follow [the seven rules of a great Git commit message](https://chris.beams.io/posts/git-commit/#seven-rules)

### Example of Contribution

Here’s an example workflow for a project theproject hosted on Github, which is currently in version 1.3.x. Your username is yourname and you’re submitting a basic bugfix.

#### Preparing your fork

1. Hit "Fork" on Github, creating e.g. `yourname/theproject`.
2. Clone your project: `git clone git@github.com:yourname/theproject`.
3. Create a branch: `cd theproject; git checkout -b foo-the-bars 1.3`.


#### Do your changes

1. Add `CHANGELOG` entry crediting yourself.
2. Write tests expecting the correct/fixed functionality; make sure they fail.
3. Code.
4. Run tests again, making sure they pass.
5. Commit your changes: `git commit -m "Foo the bars"`.

#### Creating PR (Pull Request)

1. Push your commit to get it back up to your fork: `git push origin HEAD`
2. Visit Github, click handy "Pull request" button that it will make upon noticing your new branch.
3. In the description field, write down issue number (if submitting code fixing an existing issue) or describe the issue + your fix (if submitting a wholly new bugfix).
4. Hit "Submit". Please be patient - the maintainers will get to you when they can.

## Sources

This contribution guide was inspired mainly by [Atom CONTRIBUTION guide](https://raw.githubusercontent.com/atom/atom/master/CONTRIBUTING.md) and [contribution-guide.org](http://contribution-guide-org.readthedocs.io/). Thanks for helping us with better contribution docs.

