# How to contribute

First of all, thanks for having interest in MCUSim and being ready to help!
:+1:

I started working on this program keeping in mind that a lot of makers and
engineers work on MCU-based circuits on a daily basis, and just using MCUSim
and sharing your opinion can help a lot!

## How to find any help

* Ask directly in IRC (#mcusim on [freenode](https://webchat.freenode.net/))
* Drop me a letter (Dmitry Salychev, darkness.bsd at gmail.com)
* Drop a letter to mailing list for users: mcusim@googlegroups.com
(you must be subscribed to post)
* Drop a letter to mailing list for developers: mcusim-dev@googlegroups.com
(you must be subscribed to post)
* Open an [issue](https://github.com/dsalychev/mcusim/issues)

## Sharing changes

I would like to mention that I do not accept pull requests. Please, submit
a signed patch following this way:

1. Clone repository.
2. Make and commit any changes with a message huge enough to explain what
you've done. Please, do not forget to sign off your commit:
`git commit -s -m "..."`.
3. Prepare a patch. Please, sign it off, too:
`git format-patch -s -1 <COMMIT_HASH>`.

_Important_: Please, do not combine more then one commit in a patch file.
It will help everyone to understand your changes clearly. If you're going
to make several commits in your local repository, consider
[squashing commits](https://stackoverflow.com/a/5201642/2667262) together.

Thanks,
Dmitry Salychev
