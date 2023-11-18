这段话的中文翻译如下：

# 小型聊天

摘要：这只是一个我为几位朋友编写的程序示例。它不知怎的演变成了一系列程序设计视频，继续我之前开始的一个项目：编写系统软件视频系列。

1. [第一集](https://www.youtube.com/watch?v=eT02gzeLmF0)，介绍基础服务器的工作原理。
2. [第二集](https://youtu.be/yogoUJ2zVYY)，使用原始终端处理编写一个简单的客户端。

可能还会有更多内容，敬请关注。

**重要：关于PR（Pull Request，拉取请求）的警告**：请注意，大多数添加功能的拉取请求将被拒绝，因为这个存储库的目的是在接下来的视频中逐步改进它。我们将在直播编码会议期间进行重构（或解释视频中需要重构的原因），引入更多库来改进程序的内部工作（如linenoise、rax等）。所以，如果你想作为练习来改进这个程序，那就去做吧！这是个好主意。但我不会在这里合并新功能，因为这个程序的目的是在视频中逐步演变。

## 现在，完整的故事：

昨天我和几位朋友聊天，他们大多是前端开发人员，对系统编程有些陌生。我们回忆起IRC（互联网中继聊天）的旧时光。我不可避免地说：编写一个非常简单的IRC服务器是每个人都应该尝试的经历（我向他们展示了我用TCL编写的实现；我很震惊我18年前写了这个：时间过得真快）。这样的程序有很多有趣的部分。一个单一的进程执行多路复用，处理客户端状态，一旦客户端有新数据，就尝试快速访问这种状态等。

但后来讨论进化了，我想，我会用C语言给你们展示一个非常简单的例子。你能写出最小的聊天服务器吗？首先，为了真正的最小化，我们不应该需要任何合适的客户端。即使不是很好，它也应该能够通过`telnet`或`nc`（netcat）工作。服务器的主要操作只是接收一些聊天行并将其发送给所有其他客户端，这有时被称为扇出操作。然而，这需要一个合适的`readline()`函数，然后是缓冲等。我们想让它更简单：让我们使用内核缓冲区作弊，假装我们每次都从客户端收到一个完整的行（这在实践中通常是真的，所以事情还算管用）。

嗯，有了这些技巧，我们甚至可以实现一个聊天室，只需要200行代码（当然，去掉空格和注释），让用户设置他们的昵称。既然我写了这个小程序作为一个例子给我的朋友们，我决定也把它推到GitHub上。

## 未来的工作

在接下来的几天里，我会继续修改这个程序，以便发展它。不同的演化步骤将根据我的系列*编写系统软件*的YouTube视频集的某一集来标记。这是我的计划（可能会改变，但或多或少就是我想要涵盖的内容）：

* 实现读写的缓冲。
* 避免使用线性数组，使用字典数据结构来保存客户端状态。
* 编写一个合适的客户端：能够处理异步事件的行编辑。
* 实现频道。
* 从select(2)转换到更高级的API。
* 为聊天实现简单的对称加密。

不同的变化将通过一个或多个YouTube视频来覆盖。这个存储库将保留完整的提交历史。