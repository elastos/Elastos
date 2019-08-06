/*!

=========================================================
* Light Bootstrap Dashboard React - v1.3.0
=========================================================

* Product Page: https://www.creative-tim.com/product/light-bootstrap-dashboard-react
* Copyright 2019 Creative Tim (https://www.creative-tim.com)
* Licensed under MIT (https://github.com/creativetimofficial/light-bootstrap-dashboard-react/blob/master/LICENSE.md)

* Coded by Creative Tim

=========================================================

* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

*/
import Dashboard from "views/Dashboard.jsx";
import UserProfile from "views/UserProfile.jsx";
import TableList from "views/TableList.jsx";
import Typography from "views/Typography.jsx";
import Icons from "views/Icons.jsx";

const dashboardRoutes = [
  {
    path: "/dashboard",
    name: "API Service 1",
    icon: "pe-7s-tools",
    component: Dashboard,
    layout: "/admin"
  },
  {
    path: "/user",
    name: "API Service 2",
    icon: "pe-7s-tools",
    component: UserProfile,
    layout: "/admin"
  },
  {
    path: "/table",
    name: "API Service 3",
    icon: "pe-7s-tools",
    component: TableList,
    layout: "/admin"
  },
  {
    path: "/typography",
    name: "API Service 4",
    icon: "pe-7s-tools",
    component: Typography,
    layout: "/admin"
  },
  {
    path: "/icons",
    name: "API Service 5",
    icon: "pe-7s-tools",
    component: Icons,
    layout: "/admin"
  }
];

export default dashboardRoutes;
