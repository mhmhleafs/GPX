// Put all onload AJAX calls here, and event listeners

let currentFilename = "";
let componentName = "";
let isRoute;

$(document).ready(function () {
    // On page-load AJAX Example
    console.log("CHECKPOINT");


    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/fileLogPanel',   //The server endpoint we are connecting to
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            console.log("CHECKPOINT2 data=", data);
            for (const json of data) {
                console.log("JSON=", json);
                $('#fileLogTable').find('tbody').append(fileLogJSONtoHTML(json));
            }
        },
        fail: function (error) {
            // Non-200 return, do something with error
            $('#blah').html("On page load, received error from server");
            console.log(error);
        }
    });

    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/dropDown',
        success: function (data) {
            console.log("dropDown success");
            for (const filename of data) {
                dropdownHandler(filename);
            }
        },
        fail: function (error) {
            console.log(error);
        }

    });

    // Event listener form example , we can use this instead explicitly listening for events
    // No redirects if possible
    $('#someform').submit(function (e) {
        $('#blah').html("Form has data: " + $('#entryBox').val());
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            //Create an object for connecting to another waypoint
        });
    });

    $(document).on('click', '.dropDownButton', function (e) {
        e.preventDefault();

        $('#gpxViewTable').find('tbody').empty();

        currentFilename = $(this).text().toString();

        $('#newRouteTitle').html("<label for=\"routeBox\">Add route to \"" + currentFilename + "\" that contains the following point:</label>");

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getFileInfo',
            data: {
                filename: currentFilename
            },
            success: function (data) {
                for (var obj of data) {
                    var string = gpxViewJSONtoHTML(obj);

                    $('#gpxViewTable').find('tbody').append(string);
                }
            },
            fail: function (error) {
                console.log(error);
            }
        });
    });

    $(document).on('click', '.otherDataLink', function (e) {
        e.preventDefault();

        componentName = $(this).text().toString();
        var sibs = $(this).parent().prev();
        if (sibs.text().toString().includes("Route")) {
            isRoute = true;
        }
        else {
            isRoute = false;
        }


        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getOtherData',
            data: {
                compname: componentName,
                filename: currentFilename
            },
            success: function (data) {
                if (data.length < 9)//no other data
                {
                    alert("Failed to find other data");
                    console.log("Failed to find other data");
                }
                else {
                    data = data.replace(/(\r\n|\n|\r)/gm, "");//accounts for newline and whatnot

                    var otherData = (JSON.parse(data))[0];

                    //console.log(otherData);
                    var toPrint = "Name: " + otherData.name + ". Value: " + otherData.value;
                    //console.log((JSON.parse(data))[0].name);
                    alert(toPrint);
                }
                //the data is the list of gpx data from a given component
            },
            fail: function (error) {
                console.log(error);
            }
        });
    });

    $(document).on('click', '.renameButton', function (e) {
        e.preventDefault();

        alert("Made Connection.");

        var newCompName = $('#renameBox').val();

        console.log(newCompName, componentName, currentFilename);

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/changeCompName',
            data: {
                compname: componentName,
                filename: currentFilename,
                newname: newCompName
            },
            success: function (data) {
                if (data == true) {
                    alert("successfully changed component name");
                    console.log("successfully changed component name");
                    //plz
                    $('#gpxViewTable').find('tbody').empty();

                    $.ajax({
                        type: 'get',
                        dataType: 'json',
                        url: '/getFileInfo',
                        data: {
                            filename: currentFilename
                        },
                        success: function (data) {
                            for (var obj of data) {
                                var string = gpxViewJSONtoHTML(obj);

                                $('#gpxViewTable').find('tbody').append(string);
                            }
                        },
                        fail: function (error) {
                            console.log(error);
                        }
                    });
                }
                else {
                    alert("failed to change component name (ensure component is selected)");
                    console.log("failed to change component name");
                }
                //the data is the list of gpx data from a given component
            },
            fail: function (error) {
                console.log(error);
            }
        });

    });

    $(document).on('click', '#uploadButton', function (e) {
        //e.preventDefault();

        console.log("IN BUTTON");
        //Useless \/ \/ \/
        $.ajax({
            type: 'post',
            dataType: 'json',
            url: '/upload',
            success: function (data) {
                console.log("success");
                //alert(data);
                //the data is the list of gpx data from a given component
            },
            fail: function (error) {
                console.log("failure");
                console.log(error);
            }
        });

    });

    $(document).on('click', '.addRouteButton', function (e) {
        console.log("IN ROUTE BOX");
        e.preventDefault();

        if (currentFilename == "") {
            alert("Error: no file selected");
        }
        else {


            var newLat = $('#routeBox1').val();
            var newLon = $('#routeBox2').val();
            var newName = $('#routeBox3').val();

            var newLat2 = $('#routeBox1b').val();
            var newLon2 = $('#routeBox2b').val();
            //var newName2 = $('#routeBox3b').val();

            newName = newName.replace(/(\r\n|\n|\r)/gm, "");
            //newName2 = newName2.replace(/(\r\n|\n|\r)/gm, "");

            console.log("Lat=", newLat);
            console.log("Lat2=", newLat2);

            if (isNaN(newLat) || isNaN(newLon) || isNaN(newLat2) || isNaN(newLon2)) {
                alert("Please enter numbers")
            }
            else if (newLat > 90 || newLat < -90 || newLon > 180 || newLon < -180 || newLat2 > 90 || newLat2 < -90 || newLon2 > 180 || newLon2 < -180) {
                alert("Values must be between 90 and -90 for lat and between 180 and -180 for lon")
            }
            else {
                $.ajax({
                    type: 'get',
                    dataType: 'json',
                    url: '/addRoute',
                    data: {
                        lat1: newLat,
                        lon1: newLon,
                        name1: newName,
                        lat2: newLat2,
                        lon2: newLon2,
                        filename: currentFilename,
                    },
                    success: function (data) {
                        if(data == false)
                        {
                            alert("Ensure you're logged in and have given the route a unique name");
                            return;
                        }
        
                        $('#gpxViewTable').find('tbody').append(gpxViewJSONtoHTML(data));
        
                        alert("Successfully added a route");
                    },
                    fail: function (error) {
                        alert("fail");
                        console.log(error);
                    }
                });

                //alert("Successfully added point to route");
            }
        }
    });


    $(document).on('click', '.newDocButton', function (e) {
        console.log("IN NEWDOC BOX");
        e.preventDefault();

        var docName = $('#routeBox0').val();

        docName = docName.replace(/(\r\n|\n|\r)/gm, "");

        if (docName.endsWith(".gpx") == false) {
            alert("File name must end in \".gpx\"");
        }

        currentFilename = docName;

        //$('#newRouteTitle').html("<label for=\"routeBox\">Add route to \"" + currentFilename + "\" that contains the following point:</label>");

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/addNewDoc',
            data: {
                filename: currentFilename,
            },
            success: function (data) {
                if(data == false)
                {
                    alert("You must log in before adding a doc");
                    return;
                }

                $('#fileLogTable').find('tbody').append(fileLogJSONtoHTML(data));
                
                dropdownHandler(currentFilename);

                alert("Successfully created new document");

            }

        });


    });

    //A4 BELOW \/\/\//\/\/\/\/\/

    $(document).on('click', '#loginButton', function (e) {
        e.preventDefault();

        var username = $('#usernameLogin').val();

        var password = $('#passwordLogin').val();

        var database = $('#databaseLogin').val();

        console.log(username, password, database);

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/databaseLogin',
            data: {
                uname: username,
                pword: password,
                dbase: database
            },
            success: function (data) {
                if (data == true) {
                    alert("Successfully logged in.");

                    $.ajax({
                        type: 'get',
                        dataType: 'json',
                        url: '/populateTables',
                        data: {
                            uname: username,
                            pword: password,
                            dbase: database
                        },
                        success: function (data) {
                            alert(data);
                        },
                        fail: function (error) {
                            console.log(error);
                        }
                    });
                }
                else {
                    alert("Failed to log in, please re-enter credentials");
                }
            }
        });
    });

    $(document).on('click', '#clearData', function (e) {
        e.preventDefault();

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/clearData',
            success: function (data) {
                if (data) {
                    alert("Successfully cleared all tables");
                }
                else {
                    alert("Failed to clear tables");
                }
            },
            fail: function (error) {
                console.log(error);
            }
        });

    });

    $(document).on('click', '#dbStatus', function (e) {
        e.preventDefault();

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/dbStatus',
            success: function (data) {
                if (data.status) {
                    alert(data.message);
                }
                else {
                    alert("Failed to get db status");
                }
            },
            fail: function (error) {
                console.log(error);
            }
        });

    });

    $(document).on('click', '.dropDownButton2', function (e) {
        e.preventDefault();

        $('#queryTable').find('tbody').empty();

        var text = $(this).text().toString();

        console.log(text);

        var queryNo;

        if (text == "Display routes (order by name)") {
            queryNo = 0;
        }
        else if (text == "Display routes (order by length)") {
            queryNo = 1;
        }
        else if (text == "Display routes from file (order by name, file selected from dropdown above)") {
            queryNo = 2;
        }
        else if (text == "Display routes from file (order by length, file selected from dropdown above)") {
            queryNo = 3;
        }
        else if (text == "Display points from route (select route from GPX view panel by clicking on its name)") {
            if (isRoute == false) {
                alert("Must select a route, not a track");
                return;
            }
            queryNo = 4;
        }
        else if (text == "Display points from file (order by route name, file selected from dropdown above)") {
            queryNo = 5;
        }
        else if (text == "Display points from file (order by route length, file selected from dropdown above)") {
            queryNo = 6;
        }
        else if (text == "TODO: Query 5") {
            queryNo = 7;
        }
        else {
            queryNo = -1;
        }

        console.log(queryNo);

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/query',
            data: {
                queryNo: queryNo,
                filename: currentFilename,
                componentName: componentName
            },
            success: function (data) {
                appendToQueryTable(data);
            },
            fail: function (error) {
                console.log(error);
            }
        });

    });


});//end of document.ready

//helper functions
function gpxViewJSONtoHTML(obj) {
    var component = obj.component;
    var name = obj.name;
    if (obj.name == "None") {
        console.log("NO NAME");
        obj.name = "";
    }
    var numPoints = obj.numPoints.toString();
    var len = obj.len.toString();
    var loop = obj.loop.toString();

    var html = "<tr><td>" + component + "</td>"
        + "<td><a href=\"#\" class=\"otherDataLink renameLink\">" + name + "</a></td>"
        + "<td>" + numPoints + "</td>"
        + "<td>" + len + "</td>"
        + "<td>" + loop + "</td></tr>";

    return html;
}

function fileLogJSONtoHTML(json) {
    var obj = JSON.parse(json);

    var html = "<tr><td><a href=\"" + obj.fileName + "\" download>" + obj.fileName.replace("./uploads/", "") + "</a></td>"
        + "<td>" + obj.version.toString() + "</td>"
        + "<td>" + obj.creator + "</td>"
        + "<td>" + obj.numWaypoints.toString() + "</td>"
        + "<td>" + obj.numRoutes.toString() + "</td>"
        + "<td>" + obj.numTracks.toString() + "</td></tr>";


    return html;
}

function dropdownHandler(filename) {
    $('#dropdowncontent').append("<button class=\"dropDownButton\" type=\"button\">" + filename + "</button>");
}

function appendToQueryTable(retObj) {
    if (retObj.q0 != null) {
        for (var i = 0; i < retObj.q0.length; i++) {
            var html = "<tr><td>" + retObj.q0[i].routeName + "</td><td>" + retObj.q0[i].routeLen + "</td><td>" + "none" + "</td><td>" + "none" + "</td><td>" + "none" + "</td></tr>";

            $('#queryTable').find("tbody").append(html);
        }
    }
    else if (retObj.q2 != null) {
        for (var i = 0; i < retObj.q2.length; i++) {
            var html = "<tr><td>" + retObj.q2[i].routeName + "</td><td>" + retObj.q2[i].routeLen + "</td><td>" + "none" + "</td><td>" + "none" + "</td><td>" + retObj.q2[i].filename + "</td></tr>";

            $('#queryTable').find("tbody").append(html);
        }
    }
    else if (retObj.q4 != null) {
        for (var i = 0; i < retObj.q4.length; i++) {
            var html = "<tr><td>" + "none" + "</td><td>" + "none" + "</td><td>" + retObj.q4[i].pointName + "</td><td>" + retObj.q4[i].pointIndex + "</td><td>" + "none" + "</td></tr>";

            $('#queryTable').find("tbody").append(html);
        }
    }
    else if (retObj.q5 != null) {
        for (var i = 0; i < retObj.q5.length; i++) {
            var html = "<tr><td>" + retObj.q5[i].routeName + "</td><td>" + retObj.q5[i].routeLen + "</td><td>" + retObj.q5[i].pointName + "</td><td>" + retObj.q5[i].pointIndex + "</td><td>" + retObj.q5[i].filename + "</td></tr>";

            $('#queryTable').find("tbody").append(html);
        }
    }
}