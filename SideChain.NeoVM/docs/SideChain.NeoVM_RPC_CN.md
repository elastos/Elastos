#SideChain.NeoVM侧链新增RPC接口

#介绍
为了让智能合约开发者，在不用构造交易，并等待矿工打包交易的情况下有个合约执行环境，我们在NeoVM的侧链中增加了三个RPC接口，以方便开发者测试合约。

**需要说明的是：在这三个接口中合约的执行不会产生存储，也不会构造交易。所以如果涉及到读取交易的部分，相应的RPC接口可能会执行失败**。

-------------------------------
#invokescript

***此接口方法用于直接在虚拟机中执行相应的合约代码***。也就是可以通过此接口直接执行AVM文件里的代码内容。这个RPC调用不会产生数据存储，所以不会对合约和区块链产生任何影响。

**注意：这个接口执行的代码最好是确定性的，不带合约参数的合约。**。

##参数说明

script:一个由虚拟机运行的合约脚本，也就是合约生成的avm文件中的内容。格式是由二进制转成的十六进制字串。

returnType:该合约脚本执行后返回的结果的数据类型。无返回则为"Void"。(数据类型为字符串)

下面以一个返回“hello world"的简单合约进行下示例说明：

```C#
namespace Hello{    public class Contract1 : SmartContract    {        public static String Main()        {            return "hello world";        }    }}
```
此合约生成的***script***脚本为: ***51c56b610b68656c6c6f20776f726c646c766b00527ac46203006c766b00c3616c7566***

##调用示例

```
{
  "method":"invokescript",
  "params":{"script":"51c56b610b68656c6c6f20776f726c646c766b00527ac46203006c766b00c3616c7566",
	"returnType":"String"
  }
}
```

上面示例的意思是：通过invokescript接口执行合约代码***script***为: "51c56b610b68656c6c6f20776f726c646c766b00527ac46203006c766b00c3616c7566"的脚本。该脚本的返回结果类型为***String***。

##响应正文

```
{
    "id": null,
    "jsonrpc": "2.0",
    "result": {
        "descript": "contract execution finished。",
        "gas_consumed": "0.00150000",
        "result": "hello world",
        "state": 1
    },
    "error": null
}
```

现在我们对这个接口的返回数据进行下介绍：
state：合约执行的结果状态码

		1:合约正常执行结束。表示合约按相应逻辑执行完毕。
		2:表示合约执行遇到异常，执行失败。
		4:表示合约执行中断。
		0:无状态。合约正常运行中。

descript：是对state进行的简单文字说明

gas_consumed: 合约执行的gas消耗 。以ELA为单位。

result: 该合约方法返回的结果

**descript**:合约正常执行结束，状态**state**为1.执行结果**result**为：“hello world”。执行这个合约需要消耗**gas_consumed:** 0.00150000 ELA

------------------------------------------------

#invokefunction

***此接口方法用于调用已经布署在区块链上合约***。并根据提供的参数可以调用合约内部的方法，用于查询一些数据和相应方法的gas消耗。这个RPC调用不会产生数据存储，所以不会对合约和区块链产生任何影响。

##参数说明
scripthash: 已经布署的智能合约的hash值。

operation: 合约的方法对应的操作名称。也就是你想要调用的合约的方法对应的操作名字。(数据类型为字符串)

returnType: 该合约方法所返回的数据类型。无返回则为"Void"。(数据类型为字符串)（数据类型会另有文档）

params: 该合约方法对应的参数。（此参数的数据类型为数组类型，是为了适应不同方法的参数个数和数据类型的不同，详情看下面的示例。）

我们以一个简单计算器的合约示例进行说明：

```C#
using Neo.SmartContract.Framework;using Neo.SmartContract.Framework.Services.Neo;using System;using System.Numerics;namespace Hello{    public class Contract1 : SmartContract    {        public static int Main(string operation, object[] args)        {            if (args.Length != 2)            {                throw new InvalidOperationException("args is input two integer");            }            int a = (int)args[0];            int b = (int)args[1];            if (operation == "add")            {                return Add(a, b);            }            else if (operation == "minus")            {                return Minus(a, b);            }            else if (operation == "mul")            {                return Mul(a, b);            }            else if(operation == "div")            {                return Div(a, b);            }            else            {                throw new InvalidOperationException("error operation method");            }        }        public static int Add(int a, int b)        {            return a + b;        }        public static int Minus(int a, int b)        {            return a - b;        }        public static int Mul(int a, int b)        {            return a * b;        }        public static int Div(int a, int b)        {            return a / b;        }    }}
```

这个合约生成的合约hash是:"1cc51891bfe556c00023ad5a96101caabfc210986d",将这个合约布署到区块链上后，就可以通过invokefunction接口对这个合约方法进行调用。

##调用示例
```
{
  "method":"invokefunction",
  "params":{
  "scripthash":"1cc51891bfe556c00023ad5a96101caabfc210986d",
  "operation": "add",
   "params":[
   	  {"type":"Array", "value":[{"type":"Integer","value":6},{"type":"Integer","value":2}]}
   ],
    "returnType":"Integer"
  }
}

```
上面的示例的意思是：通过invokefunction接口调用hash**(scripthash)**为“1cc51891bfe556c00023ad5a96101caabfc210986d”的合约**add**方法。该方法需要传的参数**params**为一个数组,类型***type***为Array。***value***是[{"type":"Integer","value":6},{"type":"Integer","value":2}]}
   ]。数组也是一个嵌套的参数对象。返回类型为**Integer**.**也就是计算 6 + 2 的和**。

##响应正文
```
{
    "id": null,
    "jsonrpc": "2.0",
    "result": {
        "descript": "contract execution finished。",
        "gas_consumed": "0.01320000",
        "result": 8,
        "state": 1
    },
    "error": null
}
```

现在我们对这个接口的返回数据进行下介绍：

state：合约执行的结果状态码

		1:合约正常执行结束。表示合约按相应逻辑执行完毕。
		2:表示合约执行遇到异常，执行失败。
		4:表示合约执行中断。
		0:无状态。合约正常运行中。

descript：是对state进行的简单文字说明

gas_consumed: 合约执行的gas消耗 。以ELA为单位。

result: 该合约方法返回的结果

这个接口的返回的意思是：**descript**:合约正常执行结束，状态**state**为1。执行结果**result**为：8。执行这个方法需要消耗**gas_consumed:** 0.01320000 ELA



------------------

#getOpPrice

***此接口方法用于查询虚拟机中操作指令的gas消耗***。包括基本指令OpCode和系统指令Syscall。

**注意:**

1.有些指令是需要带参数的。比如：Neo.Storage.Put, 是按需要存储的内容字节数进行计价的。包括key和value。所以需要传递两个参数,多个参数用数组表示，单个参数直接用相应的数据表示，没参数的可以忽略args参数。

2.其中需要带参数的指令包括三个,我们会对这三个做示例说明：

    "Neo.Storage.Put"
    “Neo.Asset.Renew”
    “CHECKMULTISIG”

3.NEOVM在执行完代码的时候，会在代码末尾增加一个RET指令。所以测出的gas消耗可能会比实际多0.001ELA




##参数说明

op:虚拟机的操作指令

args:操作指令对应的参数。如果没参数可以不填或忽略。

下面用两个示例说明下：

##调用示例
1.

```
{
  "method":"getOpPrice",
  "params":{
  "op":"Neo.Storage.Put",
  "args":["key1","value1"]
  }
}

```

上面示例的意思是：通过getOpPrice接口查询指令***op***为"Neo.Storage.Put"的gas消耗。需要存储的内容***args***为字符串“key1"和"value1"。

2.

```
{
  "method":"getOpPrice",
  "params":{
  "op":"CHECKMULTISIG",
  "args":3
  }
}
```

上面示例的意思是：通过getOpPrice接口查询多签指令***op***为“CHECKMULTISIG”的gas消耗。需要签名的个数***args***为3。

3.

```
{
  "method":"getOpPrice",
  "params":{
  "op":"Neo.Asset.Renew",
  "args":10
  }
}

```

上面示例的意思是：通过getOpPrice接口查询多签指令***op***为“Neo.Asset.Renew”的gas消耗。需要延长的高度***args***为10。

4.

```
{
  "method":"getOpPrice",
  "params":{
  "op":"PUSH10"
  }
}
```

上面示例的意思是：通过getOpPrice接口查询多签指令***op***为“PUSH10”的gas消耗。***没有参数，不用填,也可以忽略***

**下面的响应正文为上面调用示例的返回。**
##响应正文

1.

```
{
    "id": null,
    "jsonrpc": "2.0",
    "result": {
        "gas_consumed": "0.10010000"
    },
    "error": null
}

```

2.

```
{
    "id": null,
    "jsonrpc": "2.0",
    "result": {
        "gas_consumed": "0.03000000"
    },
    "error": null
}
```

3.

```
{
    "id": null,
    "jsonrpc": "2.0",
    "result": {
        "gas_consumed": "5000.00010000"
    },
    "error": null
}
```

4.

```
{
    "id": null,
    "jsonrpc": "2.0",
    "result": {
        "gas_consumed": "0.00010000"
    },
    "error": null
}
```

上面的响应分别返回了上面相应的rpc接口的调用。其中“Neo.Storage.Put”返回的gas消耗***gas_consumed***为0.10010000 ELA, “CHECKMULTISIG”返回的消耗为0.03000000 ELA, "Neo.Asset.Renew"的消耗为5000.00010000 ELA,"PUSH10" 的消耗为0.00010000 ELA

大家可以改下相应的参数进行测试。