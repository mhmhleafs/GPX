"use strict";

// C library API
const ffi = require("ffi-napi");

const sql = require("mysql2/promise");

const parser = ffi.Library("./libgpxparser.so", {
  gpxFileToJSON: ["string", ["string"]],
  validGPXFile: ["bool", ["string", "string"]],
  docCompToJSON: ["string", ["string"]],
  getOtherDataAsJSON: ["string", ["string", "string"]],
  changeCompName: ["bool", ["string", "string", "string"]],
  addRouteToFile: [
    "string",
    ["string", "double", "double", "double", "double", "string", "string"],
  ],
  newGPXdoc: ["bool", ["string"]],
  routesToJSON: ["string", ["string"]],
  pointsToJSON: ["string", ["string", "string"]],
  renameRoutes: ["bool", ["string"]]
});

let connection;

// Express App (Routes)
const express = require("express");
const app = express();
const path = require("path");
const fileUpload = require("express-fileupload");

app.use(fileUpload());
app.use(express.static(path.join(__dirname + "/uploads")));

// Minimization
const fs = require("fs");
const JavaScriptObfuscator = require("javascript-obfuscator");

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get("/", function (req, res) {
  res.sendFile(path.join(__dirname + "/public/index.html"));
});

// Send Style, do not change
app.get("/style.css", function (req, res) {
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname + "/public/style.css"));
});

// Send obfuscated JS, do not change
app.get("/index.js", function (req, res) {
  fs.readFile(
    path.join(__dirname + "/public/index.js"),
    "utf8",
    function (err, contents) {
      const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {
        compact: true,
        controlFlowFlattening: true,
      });
      res.contentType("application/javascript");
      res.send(minimizedContents._obfuscatedCode);
    }
  );
});

//Respond to POST requests that upload files to uploads/ directory
app.post("/upload", function (req, res) {
  if (!req.files) {
    return res.status(400).send("No files were uploaded.");
  }

  let uploadFile = req.files.uploadFile;

  var filename = uploadFile.name;

  var fileDoesExist = fileExists(filename);

  if (fileDoesExist == true) {
    //res.redirect('/');
    res.send("File already exists");
  } else {
    var pathToFiles = path.join(__dirname + "/uploads/");

    //var fileNames = fs.readdirSync(pathToFiles);
    var schema = path.join(__dirname + "/parser/gpx.xsd");
    //var schema = getSchema(fileNames);

    // Use the mv() method to place the file somewhere on your server
    uploadFile.mv("uploads/" + filename, function (err) {
      if (err) {
        return res.status(500).send(err);
      }

      var isValid = parser.validGPXFile("./uploads/" + filename, schema);

      if (isValid == false) {
        fs.unlinkSync("./uploads/" + filename);
        res.send("Invalid file");
      }

      res.redirect("/");
    });
  }
});

//Respond to GET requests for files in the uploads/ directory
app.get("/uploads/:name", function (req, res) {
  fs.stat("uploads/" + req.params.name, function (err, stat) {
    if (err == null) {
      res.sendFile(path.join(__dirname + "/uploads/" + req.params.name));
    } else {
      console.log("Error in file downloading route: " + err);
      res.send("");
    }
  });
});

//******************** Your code goes here ********************

function getGPXMetadata() {
  var pathToFiles = path.join(__dirname + "/uploads/");

  var fileNames = fs.readdirSync(pathToFiles);

  var jsonArray = [];

  var schemaFile = path.join(__dirname + "/parser/gpx.xsd");

  //schemaFile = getSchema(fileNames);

  for (const fileName of fileNames) {
    if (fileName.includes(".xsd")) {
      continue;
    }
    var json = parser.gpxFileToJSON("./uploads/" + fileName);

    if (json != "{}" && fileName.includes(".gpx")) {
      if (parser.validGPXFile("./uploads/" + fileName, schemaFile) == true) {
        jsonArray.push(json);
      }
    }
  }
  return jsonArray;
}

app.get("/fileLogPanel", function (req, res) {
  var jsonArray = getGPXMetadata();

  res.send(jsonArray);
});

app.get("/dropDown", function (req, res) {
  var pathToFiles = path.join(__dirname + "/uploads/");

  var fileNames = fs.readdirSync(pathToFiles);

  var validFiles = [];

  //var schemaFile = "";
  var schemaFile = path.join(__dirname + "/parser/gpx.xsd");
  //schemaFile = getSchema(fileNames);

  for (const filename of fileNames) {
    if (parser.validGPXFile("./uploads/" + filename, schemaFile) == true) {
      validFiles.push(filename);
    }
  }

  res.send(validFiles);
});

app.get("/getFileInfo", function (req, res) {
  var pathToFile = "./uploads/" + req.query.filename;

  var array = parser.docCompToJSON(pathToFile);

  res.send(array);
});

app.get("/getOtherData", function (req, res) {
  var pathToFile = "./uploads/" + req.query.filename;
  var compName = req.query.compname;

  var otherData = parser.getOtherDataAsJSON(pathToFile, compName);

  var string = JSON.stringify(otherData);

  res.send(string);
});

app.get("/changeCompName", async function (req, res) {
  var pathToFile = "./uploads/" + req.query.filename;
  var compName = req.query.compname;
  var newName = req.query.newname;

  var isWritten = parser.changeCompName(pathToFile, compName, newName);

  const [rows, fields] = await connection.execute(
    "SELECT gpx_id FROM FILE WHERE file_name = '" + req.query.filename + "';"
  );

  var gpxID = rows[0].gpx_id;

  connection.execute(
    "UPDATE ROUTE SET route_name = '" +
    newName +
    "' WHERE route_name = '" +
    compName +
    "' AND gpx_id = " + gpxID + ";"
  );

  const [rows1, fields1] = await connection.execute(
    "SELECT route_name FROM ROUTE;"
  );

  console.log(rows1);

  res.send(isWritten);
});

app.get("/addNewDoc", async function (req, res) {
  try {
    var pathToFile = "./uploads/" + req.query.filename;
    parser.newGPXdoc(pathToFile);

    var json = parser.gpxFileToJSON(pathToFile);

    var sql = jsonToSQLFileTable(json);

    await connection.execute(sql);

    res.send(JSON.stringify(json));
  } catch (e) {
    res.send(false);
  }
});

app.get("/addRoute", async function (req, res) {
  try {
    var pathToFile = "./uploads/" + req.query.filename;

    var latA = req.query.lat1;
    var lonA = req.query.lon1;
    var nameA = req.query.name1;
    var latB = req.query.lat2;
    var lonB = req.query.lon2;

    var [rows, fields] = await connection.execute("SELECT gpx_id FROM FILE WHERE file_name = '" + req.query.filename + "';");

    var gpxID = rows[0].gpx_id;

    [rows, fields] = await connection.execute("SELECT route_name FROM ROUTE WHERE gpx_id = " + gpxID + ";");

    for (var row of rows) {
      if (row.route_name == nameA) {
        res.send(false);
      }
    }

    var isAdded = parser.addRouteToFile(
      pathToFile,
      latA,
      lonA,
      latB,
      lonB,
      nameA,
      "dummy"
    );

    console.log(typeof isAdded);
    console.log(isAdded);

    var parsed = JSON.parse(isAdded);

    [rows, fields] = await connection.execute("SELECT COUNT(*) AS numRoutes FROM ROUTE WHERE gpx_id = " + gpxID + ";");

    console.log(rows[0]);

    var numRoutes = rows[0].numRoutes;

    var routeTotal = numRoutes + 1;

    parsed.component = "Route " + routeTotal;

    var sql = jsonToSQLRouteTable(parsed, gpxID);

    await connection.execute(sql);

    console.log(nameA);

    [rows, fields] = await connection.execute("SELECT route_id FROM ROUTE WHERE route_name = '" + nameA + "';");

    var routeID = rows[0].route_id;

    console.log(routeID);


    var wpt1 = {
      name: "null",
      latitude: latA,
      longitude: lonA,
      index: 0
    }

    var wpt2 = {
      name: "null",
      latitude: latB,
      longitude: lonB,
      index: 1
    }

    sql = jsonToSQLPointTable(wpt1, routeID);

    await connection.execute(sql);

    sql = jsonToSQLPointTable(wpt2, routeID);

    await connection.execute(sql);

    res.send(parsed);
  }
  catch (e) {
    res.send(false);
  }
});

//A4 BELOW \/\/\/\/\/\/\/\/

app.get("/databaseLogin", async function (req, res) {
  connection = await sql.createConnection({
    host: "dursley.socs.uoguelph.ca",
    user: req.query.uname,
    password: req.query.pword,
    database: req.query.dbase,
  });

  createTables();

  await connection.execute("DELETE FROM FILE;");

  await connection.execute("DELETE FROM ROUTE;");

  await connection.execute("DELETE FROM POINT;");

  console.log("LOGIN SUCCEEDED");
  res.send(true);
});

app.get("/populateTables", async function (req, res) {
  var pathToFiles = path.join(__dirname + "/uploads/");

  var fileNames = fs.readdirSync(pathToFiles);

  //var schemaFile = "";
  var schemaFile = path.join(__dirname + "/parser/gpx.xsd");
  //schemaFile = getSchema(fileNames);

  for (var filename of fileNames) {
    if (parser.validGPXFile("./uploads/" + filename, schemaFile) == true) {
      var json = parser.gpxFileToJSON("./uploads/" + filename);

      var sql = jsonToSQLFileTable(json);

      await connection.execute(sql);

      const [rows, fields] = await connection.query(
        'SELECT gpx_id FROM FILE WHERE file_name = "' + filename + '";'
      );

      var routes = parser.routesToJSON("./uploads/" + filename);

      routes = JSON.parse(routes);

      var gpxID = rows[0].gpx_id;

      for (var route of routes) {
        sql = jsonToSQLRouteTable(route, gpxID);

        await connection.execute(sql);

        var points = parser.pointsToJSON(route.name, "./uploads/" + filename);

        points = JSON.parse(points);

        const [rows1, fields1] = await connection.query(
          "SELECT route_id FROM ROUTE WHERE route_name = '" + route.name + "';"
        );

        console.log(rows1[0].route_id);

        var routeID = rows1[0].route_id;

        for (var point of points) {
          sql = jsonToSQLPointTable(point, routeID);

          connection.execute(sql);
          //create sql to insert waypoints into POINT table based on route id
        }
      }
    }
  }

  res.send(true);
});

app.get("/clearData", async function (req, res) {
  connection.execute("DELETE FROM POINT;");
  connection.execute("DELETE FROM ROUTE;");
  connection.execute("DELETE FROM FILE;");

  var [rows1, fields1] = await connection.query("SELECT * FROM POINT;");

  console.log("EXPECTING EMPTY POINT TABLE");
  console.log(rows1);

  [rows1, fields1] = await connection.query("SELECT * FROM ROUTE;");
  console.log("EXPECTING EMPTY ROUTE TABLE");
  console.log(rows1);

  [rows1, fields1] = await connection.query("SELECT * FROM FILE;");
  console.log("EXPECTING EMPTY FILE TABLE");
  console.log(rows1);

  res.send(true);
});

app.get("/dbStatus", async function (req, res) {
  var [rows, fields] = await connection.execute(
    "SELECT COUNT(*) AS COUNT FROM FILE;"
  );
  var [rows1, fields1] = await connection.execute(
    "SELECT COUNT(*) AS COUNT FROM ROUTE;"
  );
  var [rows2, fields2] = await connection.execute(
    "SELECT COUNT(*) AS COUNT FROM POINT;"
  );

  var obj = {
    status: true,
    message:
      "Database has " +
      rows[0].COUNT +
      " files, " +
      rows1[0].COUNT +
      " routes, and " +
      rows2[0].COUNT +
      " points",
  };

  res.send(obj);
});

app.get("/query", async function (req, res) {
  var queryNumber = req.query.queryNo;

  var filename = req.query.filename;

  var componentName = req.query.componentName;

  var returnObject = {
    q0: null,
    q1: null,
    q2: null,
    q3: null,
    q4: null,
    q5: null,
    q6: null,
  };

  if (queryNumber == 0) {
    const [rows, fields] = await connection.query(
      "SELECT route_name, route_len FROM ROUTE ORDER BY route_name;"
    );

    var filenames = [];

    for (var i = 0; i < rows.length; i++) {
      var obj = {
        routeName: rows[i].route_name,
        routeLen: rows[i].route_len,
        pointName: null,
        pointIndex: null,
        filename: null,
      };

      filenames.push(obj);
    }

    returnObject.q0 = filenames;
  } else if (queryNumber == 1) {
    const [rows, fields] = await connection.query(
      "SELECT route_name, route_len FROM ROUTE ORDER BY route_len DESC;"
    );

    var filenames = [];

    for (var i = 0; i < rows.length; i++) {
      var obj = {
        routeName: rows[i].route_name,
        routeLen: rows[i].route_len,
        pointName: null,
        pointIndex: null,
        filename: null,
      };

      filenames.push(obj);
    }

    returnObject.q0 = filenames;
  } else if (queryNumber == 2) {
    const [r, f] = await connection.query(
      "SELECT gpx_id FROM FILE WHERE file_name = '" + filename + "';"
    );

    var gpxID = r[0].gpx_id;

    const [rows, fields] = await connection.query(
      "SELECT route_name, route_len FROM ROUTE WHERE gpx_id = " +
      gpxID +
      " ORDER BY route_name;"
    );

    var filenames = [];

    for (var i = 0; i < rows.length; i++) {
      var obj = {
        routeName: rows[i].route_name,
        routeLen: rows[i].route_len,
        pointName: null,
        pointIndex: null,
        filename: filename,
      };

      filenames.push(obj);
    }

    returnObject.q2 = filenames;
  } else if (queryNumber == 3) {
    const [r, f] = await connection.query(
      "SELECT gpx_id FROM FILE WHERE file_name = '" + filename + "';"
    );

    var gpxID = r[0].gpx_id;

    const [rows, fields] = await connection.query(
      "SELECT route_name, route_len FROM ROUTE WHERE gpx_id = " +
      gpxID +
      " ORDER BY route_len DESC;"
    );

    var filenames = [];

    for (var i = 0; i < rows.length; i++) {
      var obj = {
        routeName: rows[i].route_name,
        routeLen: rows[i].route_len,
        pointName: null,
        pointIndex: null,
        filename: filename,
      };

      filenames.push(obj);
    }

    returnObject.q2 = filenames;
  } else if (queryNumber == 4) {
    const [r, f] = await connection.query(
      "SELECT gpx_id FROM FILE WHERE file_name = '" + filename + "';"
    );

    var gpxID = r[0].gpx_id;

    const [rows, fields] = await connection.query(
      "SELECT route_id FROM ROUTE WHERE gpx_id = " +
      gpxID +
      " AND route_name = '" + componentName + "';"
    );

    var routeID = rows[0].route_id;

    const [rows2, fields2] = await connection.query(
      "SELECT point_name, point_index FROM POINT WHERE route_id = " +
      routeID + " ORDER BY point_index;"
    );

    var filenames = [];

    for (var i = 0; i < rows2.length; i++) {
      var obj = {
        routeName: null,
        routeLen: null,
        pointName: rows2[i].point_name,
        pointIndex: rows2[i].point_index,
        filename: null,
      };

      filenames.push(obj);
    }

    returnObject.q4 = filenames;
  } else if (queryNumber == 5) {
    var [r, f] = await connection.query(
      "SELECT gpx_id FROM FILE WHERE file_name = '" + filename + "';"
    );

    var gpxID = r[0].gpx_id;

    var [rows, fields] = await connection.query(
      "SELECT route_id, route_len, route_name FROM ROUTE WHERE gpx_id = " +
      gpxID + " ORDER BY route_name;"
    );

    var filenames = [];

    for (var i = 0; i < rows.length; i++) {
      var routeID = rows[i].route_id;

      var [rows2, fields2] = await connection.query(
        "SELECT point_name, point_index FROM POINT WHERE route_id = " +
        routeID + " ORDER BY point_index;"
      );

      console.log("ROWS2 len = " + rows2.length.toString());


      for (var k = 0; k < rows2.length; k++) {
        var obj = {
          routeName: rows[i].route_name,
          routeLen: rows[i].route_len,
          pointName: rows2[k].point_name,
          pointIndex: rows2[k].point_index,
          filename: filename,
        };

        filenames.push(obj);
      }

    }

    returnObject.q5 = filenames;
  } else if (queryNumber == 6) {
    var [r, f] = await connection.query(
      "SELECT gpx_id FROM FILE WHERE file_name = '" + filename + "';"
    );

    var gpxID = r[0].gpx_id;

    var [rows, fields] = await connection.query(
      "SELECT route_id, route_len, route_name FROM ROUTE WHERE gpx_id = " +
      gpxID + " ORDER BY route_len DESC;"
    );

    var filenames = [];

    for (var i = 0; i < rows.length; i++) {
      var routeID = rows[i].route_id;

      var [rows2, fields2] = await connection.query(
        "SELECT point_name, point_index FROM POINT WHERE route_id = " +
        routeID + " ORDER BY point_index;"
      );

      console.log("ROWS2 len = " + rows2.length.toString());


      for (var k = 0; k < rows2.length; k++) {
        var obj = {
          routeName: rows[i].route_name,
          routeLen: rows[i].route_len,
          pointName: rows2[k].point_name,
          pointIndex: rows2[k].point_index,
          filename: filename,
        };

        filenames.push(obj);
      }

    }

    returnObject.q5 = filenames;
  }

  //console.log(returnObject);

  res.send(returnObject);
});

//Sample endpoint
app.get("/endpoint1", function (req, res) {
  console.log("Made a connection");
  let retStr = req.query.stuff + " " + req.query.junk;
  res.send({
    stuff: retStr,
  });
});

function getSchema(fileNames) {
  var schemaFile = "";

  for (const filename of fileNames) {
    if (filename.includes(".xsd")) {
      schemaFile = "./uploads/" + filename;

      return schemaFile;
    }
  }

  return null;
}

function fileExists(filename) {
  var pathToFiles = path.join(__dirname + "/uploads/");

  var fileNames = fs.readdirSync(pathToFiles);

  for (var fn of fileNames) {
    if (fn == filename) {
      return true;
    }
  }
  return false;
}

async function createTables() {
  var fileTable =
    "CREATE TABLE IF NOT EXISTS FILE (gpx_id INT AUTO_INCREMENT PRIMARY KEY, " +
    "file_name VARCHAR(60) NOT NULL, " +
    "ver  DECIMAL(2,1) NOT NULL, " +
    "creator  VARCHAR(256) NOT NULL);";

  await connection.execute(fileTable);

  var routeTable =
    "CREATE TABLE IF NOT EXISTS ROUTE (route_id INT AUTO_INCREMENT PRIMARY KEY, " +
    "route_name VARCHAR(256), " +
    "route_len FLOAT(15,7) NOT NULL, " +
    "gpx_id INT NOT NULL, " +
    "FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE);";

  await connection.execute(routeTable);

  var pointTable =
    "CREATE TABLE IF NOT EXISTS POINT (" +
    "point_id INT AUTO_INCREMENT PRIMARY KEY, " +
    "point_index INT NOT NULL, " +
    "latitude DECIMAL(11,7) NOT NULL, " +
    "longitude DECIMAL(11,7) NOT NULL, " +
    "point_name VARCHAR(256), " +
    "route_id INT NOT NULL, " +
    "FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE);";

  await connection.execute(pointTable);
}

function jsonToSQLFileTable(obj) {
  obj = JSON.parse(obj);

  if (obj.fileName == undefined) {
    console.log("FILENAME UNDEFINED");
    return null;
  }
  var heading = "(file_name, ver, creator)";

  var filename = obj.fileName.replace("./uploads/", "");

  var version = obj.version;

  var creator = obj.creator;

  var values = "('" + filename + "', " + version + ", '" + creator + "')";

  var sql = "INSERT INTO FILE " + heading + " VALUES " + values + ";";

  return sql;
}

function jsonToSQLRouteTable(obj, gpxID) {
  //obj = JSON.parse(obj);

  var heading = "(route_name, route_len, gpx_id)";

  try {
    // const results = await connection.promise().query(gpxIdQuery);

    var values = "('" + obj.name + "', " + obj.len + ", " + gpxID + ")";

    var sql = "INSERT INTO ROUTE " + heading + " VALUES " + values + ";";

    return sql;
  } catch (error) {
    console.log(error);

    return null;
  }
}

function jsonToSQLPointTable(obj, routeID) {
  var heading = "(point_index, latitude, longitude, point_name, route_id)";

  var index = obj.index;

  var lat = obj.latitude;

  var lon = obj.longitude;

  var name = obj.name;

  if (name == "null") {
    name = null;
  } else {
    name = "'" + name + "'";
  }

  var values =
    "(" + index + ", " + lat + ", " + lon + ", " + name + ", " + routeID + ")";

  var sql = "INSERT INTO POINT " + heading + " VALUES " + values + ";";

  return sql;
}

function renameUnnamedRoutes() {
  var pathToFiles = path.join(__dirname + "/uploads/");

  var fileNames = fs.readdirSync(pathToFiles);

  var schemaFile = path.join(__dirname + "/parser/gpx.xsd");

  //schemaFile = getSchema(fileNames);

  for (const fileName of fileNames) {
    if (fileName.includes(".xsd")) {
      continue;
    }

    if (fileName.includes(".gpx")) {
      if (parser.validGPXFile("./uploads/" + fileName, schemaFile) == true) {
        parser.renameRoutes("./uploads/" + fileName);
      }
    }
  }
}

app.listen(portNum);
console.log("Running app at localhost: " + portNum);

renameUnnamedRoutes();

getGPXMetadata();
