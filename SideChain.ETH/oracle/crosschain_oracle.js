"use strict";

const express = require("express");

const common = require("./common");
const getBlkNum = require("./getblknum");
const sendrechargetransaction = require("./sendrechargetransaction");
const getTxInfo = require("./gettxinfo");
const getBlkLogs = require("./getblklogs");
const getExistTxs = require("./getexisttxs");
const GetIllegalEvidenceByHeight=require("./getillegalevidencebyheight");
const CheckIllegalEvidence=require("./checkillegalevidence");

const app = express();

app.use(express.json());

app.post("/", async function(req, res) {
    try {
        let json_data = req.body;
        console.log("JSON Data Received: ");
        console.log(json_data);
        console.log("============================================================");
        if (json_data["method"] === "getblockcount") {
            await getBlkNum(res);
            return;
        }
        if (json_data["method"] === "sendrechargetransaction") {
            await sendrechargetransaction(json_data, res);
            return;
        }
        if (json_data["method"] === "getwithdrawtransaction") {
            await getTxInfo(json_data, res);
            return;
        }
        if (json_data["method"] === "getwithdrawtransactionsbyheight") {
            await getBlkLogs(json_data, res);
            return;
        }
        if (json_data["method"] === "getexistdeposittransactions") {
            await getExistTxs(json_data, res);
            return;
        }
        if (json_data["method"] === "getillegalevidencebyheight") {
            await GetIllegalEvidenceByHeight(json_data, res);
             return;
    }
        if (json_data["method"] === "checkillegalevidence") {
            await CheckIllegalEvidence(json_data, res);
            return;
        }
    } catch (err) {
        common.reterr(err, res);
        return;
    }
    res.json({"result": "received"});
});

let server = app.listen('20632');
server.timeout = 360000;
console.log("Server started...");

process.on("SIGINT", () => {
    console.log("Shutting down...");
    process.exit();
});
