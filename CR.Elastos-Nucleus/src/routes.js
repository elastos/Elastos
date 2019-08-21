import Apigenerator from "views/Apigenerator.jsx";
import Dashboard from "views/Dashboard.jsx";

import ServiceOne from "views/fileupload.jsx";
import ServiceTwo from "views/ServiceTwo.jsx";
import ServiceThree from "views/ServiceThree.jsx";
import ServiceFour from "views/ServiceFour.jsx";
import ServiceFive from "views/ServiceFive.jsx";
import ServiceSix from "views/ServiceSix.jsx";

import UploadAndSign from "views/uploadandsign.jsx";
import ShowFileContent from "views/showfilecontent.jsx";

const sidebarRoutes = [
  {
    path: "/generate-api-key",
    name: "Generate API Key",
    icon: "pe-7s-tools",
    component: Apigenerator,
    layout: "/admin"
  },
  {
    path: "/file-upload",
    name: "File Upload",
    icon: "pe-7s-tools",
    component: ServiceOne,
    layout: "/admin"
  },
  {
    path: "/show-file-content",
    name: "Show File Content",
    icon: "pe-7s-tools",
    component: ShowFileContent,
    layout: "/admin"
  },
  {
    path: "/upload-and-sign",
    name: "Upload and Sign",
    icon: "pe-7s-tools",
    component: UploadAndSign,
    layout: "/admin"
  },
  {
    path: "/verify-content",
    name: "Verify and Show Content",
    icon: "pe-7s-tools",
    component: ServiceFour,
    layout: "/admin"
  },
  {
    path: "/sign-message",
    name: "Sign A Message",
    icon: "pe-7s-tools",
    component: ServiceTwo,
    layout: "/admin"
  },
  {
    path: "/verify-message",
    name: "Verify A Message",
    icon: "pe-7s-tools",
    component: ServiceThree,
    layout: "/admin"
  },
  {
    path: "/transfer-ela",
    name: "Transfer ELA Demo",
    icon: "pe-7s-tools",
    component: ServiceFive,
    layout: "/admin"
  },
  {
    path: "/documentation",
    name: "Documentation",
    icon: "pe-7s-tools",
    component: ServiceSix,
    layout: "/admin"
  },
  {
    path: "/dashboard",
    name: "Dashboard",
    icon: "pe-7s-tools",
    component: Dashboard,
    layout: "/admin"
  }
];

export default sidebarRoutes;
