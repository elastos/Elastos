import NewWallet from "views/GenerateWallet.jsx";
import Balance from "views/GetBalance.jsx";
import TransactionHistory from "views/GetTransactionHistory.jsx";
import Transactions from "views/GetTransactions.jsx";
import TransferELA from "views/TransferELA.jsx";


const appRoutes = [
  {
    path: "/generatewallet",
    name: "Create Wallet",
    icon: "pe-7s-tools",
    component: NewWallet,
    layout: "/admin"
  },
  {
    path: "/getbalance",
    name: "Wallet Balance",
    icon: "pe-7s-tools",
    component: Balance,
    layout: "/admin"
  },
  {
    path: "/transactionhistory",
    name: "Transaction History",
    icon: "pe-7s-tools",
    component: TransactionHistory,
    layout: "/admin"
  },
  {
    path: "/transactions",
    name: "Transactions List",
    icon: "pe-7s-tools",
    component: Transactions,
    layout: "/admin"
  },
  {
    path: "/transfer",
    name: "Sidechain Service",
    icon: "pe-7s-tools",
    component: TransferELA,
    layout: "/admin"
  }
];

export default appRoutes;