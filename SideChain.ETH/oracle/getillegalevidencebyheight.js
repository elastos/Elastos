"use strict";

const common = require("./common");

module.exports = async function(json_data, res) {
    try {
        let height = json_data["params"]["height"];
        console.log(height);
        console.log("============================================================");
        let result = new Array();
        res.json({"error": null, "id": null, "jsonrpc": "2.0", "result": result});
        return;
    } catch (err) {
        common.reterr(err, res);
        return;
    }
}
