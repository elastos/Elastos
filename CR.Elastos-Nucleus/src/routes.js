import ApiKeyGenerator from "views/Apigenerator.jsx";
import Dashboard from "views/Dashboard.jsx";

import FileUpload from "views/FileUpload.jsx";
import SignMessage from "views/SignMessage.jsx";

import VerifyMessage from "views/VerifyMessage.jsx";
import VerifyShowContent from "views/VerifyShowContent.jsx";

import TransferELA from "views/TransferELA.jsx";
import Documentation from "views/Documentation.jsx";

import UploadAndSign from "views/UploadAndSign.jsx";
import ShowFileContent from "views/ShowFileContent.jsx";

const sidebarRoutes = [
  {
    path: "/generate-api-key",
    name: "Generate API Key",
    icon: "pe-7s-tools",
    component: ApiKeyGenerator,
    layout: "/admin"
  },
  {
    path: "/file-upload",
    name: "File Upload",
    icon: "pe-7s-tools",
    component: FileUpload,
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
    component: VerifyShowContent,
    layout: "/admin"
  },
  {
    path: "/sign-message",
    name: "Sign A Message",
    icon: "pe-7s-tools",
    component: SignMessage,
    layout: "/admin"
  },
  {
    path: "/verify-message",
    name: "Verify A Message",
    icon: "pe-7s-tools",
    component: VerifyMessage,
    layout: "/admin"
  },
  {
    path: "/transfer-ela",
    name: "Transfer ELA Demo",
    icon: "pe-7s-tools",
    component: TransferELA,
    layout: "/admin"
  },
  {
    path: "/documentation",
    name: "Documentation",
    icon: "pe-7s-tools",
    component: Documentation,
    layout: "/admin"
  },
  {
    path: "/dashboard",
    name: "About Us",
    icon: "pe-7s-tools",
    component: Dashboard,
    layout: "/admin"
  }
];

export default sidebarRoutes;
