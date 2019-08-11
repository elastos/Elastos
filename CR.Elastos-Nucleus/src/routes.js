import Dashboard from "views/Dashboard.jsx";
import ServiceOne from "views/ServiceOne.jsx";
import ServiceTwo from "views/ServiceTwo.jsx";
import ServiceThree from "views/ServiceThree.jsx";
import ServiceFour from "views/ServiceFour.jsx";
import ServiceFive from "views/ServiceFive.jsx";

const sidebarRoutes = [
  {
    path: "/dashboard",
    name: "Dashboard",
    icon: "pe-7s-tools",
    component: Dashboard,
    layout: "/admin"
  },
  {
    path: "/serviceone",
    name: "Wallet Service",
    icon: "pe-7s-tools",
    component: ServiceOne,
    layout: "/admin"
  },
  {
    path: "/servicetwo",
    name: "DID Service",
    icon: "pe-7s-tools",
    component: ServiceTwo,
    layout: "/admin"
  },
  {
    path: "/servicethree",
    name: "Mainchain Service",
    icon: "pe-7s-tools",
    component: ServiceThree,
    layout: "/admin"
  },
  {
    path: "/servicefour",
    name: "Sidechain Service",
    icon: "pe-7s-tools",
    component: ServiceFour,
    layout: "/admin"
  },
  {
    path: "/servicefive",
    name: "Generate API Key",
    icon: "pe-7s-tools",
    component: ServiceFive,
    layout: "/admin"
  }
];

export default sidebarRoutes;
