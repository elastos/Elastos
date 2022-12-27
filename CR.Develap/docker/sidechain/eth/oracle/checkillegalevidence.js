"use strict";

const common = require("./common");

module.exports = async function(json_data, res) {
    try {
        let evidence = json_data["params"]["evidence"];
        console.log(evidence);
        console.log("============================================================");
        res.json({"error": null, "id": null, "jsonrpc": "2.0", "result": false});
        return;
    } catch (err) {
        common.reterr(err, res);
        return;
    }
}
