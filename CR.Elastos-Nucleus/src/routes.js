import Dashboard from "views/Dashboard.jsx";
import ServiceOne from "views/ServiceOne.jsx";
import ServiceTwo from "views/ServiceTwo.jsx";
import ServiceThree from "views/ServiceThree.jsx";
import ServiceFour from "views/ServiceFour.jsx";
import ServiceFive from "views/ServiceFive.jsx";
import ServiceSix from "views/ServiceSix.jsx";

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
    name: "File Upload",
    icon: "pe-7s-tools",
    component: ServiceOne,
    layout: "/admin"
  },
  {
    path: "/servicetwo",
    name: "Sign A File",
    icon: "pe-7s-tools",
    component: ServiceTwo,
    layout: "/admin"
  },
  {
    path: "/servicethree",
    name: "Verify A File",
    icon: "pe-7s-tools",
    component: ServiceThree,
    layout: "/admin"
  },
  {
    path: "/servicefour",
    name: "Verify Content",
    icon: "pe-7s-tools",
    component: ServiceFour,
    layout: "/admin"
  },
  {
    path: "/servicefive",
    name: "Transfer ELA Demo",
    icon: "pe-7s-tools",
    component: ServiceFive,
    layout: "/admin"
  },
  {
    path: "/servicesix",
    name: "Documentation",
    icon: "pe-7s-tools",
    component: ServiceSix,
    layout: "/admin"
  }
];

export default sidebarRoutes;
