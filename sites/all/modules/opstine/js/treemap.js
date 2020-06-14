(function ($) {
    Drupal.behaviors.opstine_budzet = {
        attach: function (context) {

            var datapath = Drupal.settings.opstine.datapath;
            var siteurl = Drupal.settings.opstine.siteurl;

            // Parameters
/*            var width = document.body.clientWidth * 9 / 10;
            //var height = 2*width;

            var height = document.body.clientHeight;
            $("#chart_wrapper").height(height+200);
            //var width = height/2;*/
            var temp_rate = function (){
                return Math.random()*0.2-0.1;
            };

           // var json_file = datapath + "/test.json";
            var json_file = datapath;

            var obj = document.getElementById("tree_wrapper");
            var divWidth = obj.offsetWidth;

            var margin = {top: 30, right: 0, bottom: 20, left: 0},
                width = divWidth -25,
                height = 640 - margin.top - margin.bottom,
                formatNumber = d3.format(",%"),
                colorDomain = [-.1, 0, .1],
                colorRange = ["#dda8db", "#ebf2f7", "#9cbdd9"],
                transitioning;

            // sets x and y scale to determine size of visible boxes
            var x = d3.scale.linear()
                .domain([0, width])
                .range([0, width]);

            var y = d3.scale.linear()
                .domain([0, height])
                .range([0, height]);

            // adding a color scale
            var color = d3.scale.linear()
                .domain(colorDomain)
                .range(colorRange);

            // introduce color scale here
            var treemap = d3.layout.treemap()
                .children(function(d, depth) { return depth ? null : d._children; })
                .sort(function(a, b) { return a.value - b.value; })
                .ratio(height / width * 0.5 * (1 + Math.sqrt(5)))
                .round(false);

            var svg = d3.select("#tree_wrapper").append("svg")
                .attr("width", width + margin.left + margin.right)
                .attr("height", height + margin.bottom + margin.top)
                .style("margin-left", -margin.left + "px")
                .style("margin.right", -margin.right + "px")
                .append("g")
                .attr("transform", "translate(" + margin.left + "," + margin.top + ")")
                .style("shape-rendering", "crispEdges");

            var grandparent = svg.append("g")
                .attr("class", "grandparent");

            grandparent.append("rect")
                .attr("y", -margin.top)
                .attr("width", width)
                .attr("height", margin.top);

            grandparent.append("text")
                .attr("x", 6)
                .attr("y", 6 - margin.top)
                .attr("dy", ".75em");

            var legend = d3.select("#legend").append("svg")
                .attr("width", width + margin.left + margin.right)
                .attr("height", 30)
                .attr("class", "legend")
                .selectAll("g")
                .data([0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18])
                //.data([0,2,4,5,8,9,10,11,12,14,16,18])
                .enter()
                .append("g");

            // functions
            function initialize(root) {
                root.x = root.y = 0;
                root.dx = width;
                root.dy = height;
                root.depth = 0;
            }

            // Aggregate the values for internal nodes. This is normally done by the
            // treemap layout, but not here because of our custom implementation.
            // We also take a snapshot of the original children (_children) to avoid
            // the children being overwritten when when layout is computed.
            function accumulate(d) {
                return (d._children = d.children) ? d.value = d.children.reduce(function(p, v) {return p + accumulate(v); }, 0) : d.value;
            }

            // Compute the treemap layout recursively such that each group of siblings
            // uses the same size (1×1) rather than the dimensions of the parent cell.
            // This optimizes the layout for the current zoom state. Note that a wrapper
            // object is created for the parent node for each group of siblings so that
            // the parent’s dimensions are not discarded as we recurse. Since each group
            // of sibling was laid out in 1×1, we must rescale to fit using absolute
            // coordinates. This lets us use a viewport to zoom.
            function layout(d) {
                if (d._children) {
                    // treemap nodes comes from the treemap set of functions as part of d3
                    treemap.nodes({_children: d._children});
                    d._children.forEach(function(c) {
                        c.x = d.x + c.x * d.dx;
                        c.y = d.y + c.y * d.dy;
                        c.dx *= d.dx;
                        c.dy *= d.dy;
                        c.parent = d;
                        // recursion
                        layout(c);
                    });
                }
            }

            function colorIncrements(d){
                return (colorDomain[colorDomain.length - 1] - colorDomain[0])/18*d + colorDomain[0];
            }

            legend.append("rect")
                .attr("x", function(d){return margin.left + d * 35;})
                .attr("y", 0)
                .attr("fill", function(d) {return color(colorIncrements(d));})
                .attr("width", "35px")
                .attr("height", "40px");

            legend.append("text")
                .text(function(d){return formatNumber(colorIncrements(d));})
                .attr("y", 20)
                .attr("x", function(d){return (margin.left + d * 35 + 20);});

            // determines if white or black will be better contrasting color
            function getContrast50(hexcolor){
                return (parseInt(hexcolor.replace("#", ""), 16) > 0xffffff/3) ? "black" : "white";
            }

          function display(d) {

            function text(text) {
              text.attr("x", function(d) { return x(d.x) + 6; })
                .attr("y", function(d) { return y(d.y) + 6; });
            }

            function rect(rect) {
              rect.attr("x", function(d) { return x(d.x); })
                .attr("y", function(d) { return y(d.y); })
                .attr("width", function(d) { return x(d.x + d.dx) - x(d.x); })
                .attr("height", function(d) { return y(d.y + d.dy) - y(d.y); })
                .attr("fill", function(d){return color(parseFloat(temp_rate()));});
            }

            function foreign(foreign){ /* added */
              foreign.attr("x", function(d) { return x(d.x); })
                .attr("y", function(d) { return y(d.y); })
                .attr("width", function(d) { return x(d.x + d.dx) - x(d.x); })
                .attr("height", function(d) { return y(d.y + d.dy) - y(d.y); });
            }

            function transition(d) {
              if (transitioning || !d) {return;}
              transitioning = true;

              var g2 = display(d),
                t1 = g1.transition().duration(650),
                t2 = g2.transition().duration(650);

              // Update the domain only after entering new elements.
              x.domain([d.x, d.x + d.dx]);
              y.domain([d.y, d.y + d.dy]);

              // Enable anti-aliasing during the transition.
              svg.style("shape-rendering", null);

              // Draw child nodes on top of parent nodes.
              svg.selectAll(".depth").sort(function(a, b) { return a.depth - b.depth; });

              // Fade-in entering text.
              g2.selectAll("text").style("fill-opacity", 0);
              g2.selectAll("foreignObject div").style("display", "none"); /*added*/

              // Transition to the new view.
              t1.selectAll("text").call(text).style("fill-opacity", 0);
              t2.selectAll("text").call(text).style("fill-opacity", 1);
              t1.selectAll("rect").call(rect);
              t2.selectAll("rect").call(rect);

              /* Foreign object */
              t1.selectAll(".textdiv").style("display", "none"); /* added */
              t1.selectAll(".foreignobj").call(foreign); /* added */
              t2.selectAll(".textdiv").style("display", "block"); /* added */
              t2.selectAll(".foreignobj").call(foreign); /* added */

              // Remove the old node when the transition is finished.
              t1.remove().each("end", function() {
                svg.style("shape-rendering", "crispEdges");
                transitioning = false;
              });
            }

            function name(d) {
              return d.parent ? "Буџет општине" + " - " + d.name + " - Кликни да умањиш" : "Буџет општине - кликни да увећаш";
            }

            function nameSave(d) {
              return d.parent ? name(d.parent) + " - " + d.name + " -  Кликни да умањиш" : d.name;
            }

            grandparent
              .datum(d.parent)
              .on("click", transition)
              .select("text")
              .text(name(d));

            // color header based on grandparent's rate
            grandparent
              .datum(d.parent)
              .select("rect")
              .attr("fill", function(){
                console.log(color(temp_rate()));
                return color(temp_rate());
              });

            var g1 = svg.insert("g", ".grandparent")
              .datum(d)
              .attr("class", "depth");

            var g = g1.selectAll("g")
              .data(d._children)
              .enter().append("g");

            g.filter(function(d) { return d._children; })
              .classed("children", true)
              .on("click", transition);

            g.selectAll(".child")
              .data(function(d) { return d._children || [d]; })
              .enter().append("rect")
              .attr("class", "child")
              .call(rect);

            g.append("rect")
              .attr("class", "parent")
              .call(rect)
              .append("title");

            /* Adding a foreign object instead of a text object, allows for text wrapping */
            g.append("foreignObject")
              .call(rect)
              /* open new window based on the json's URL value for leaf nodes */
              /* Firefox displays this on top
               .on("click", function(d) {
               if(!d.children){
               window.open(d.url);
               }
               })*/
              .attr("class","foreignobj")
              .append("xhtml:div")
              .attr("dy", ".75em")
              .html(function(d) { return " <p class='title'> " + d.name + "</p><p>" + d3.format(",r")(d.value) + " динара </p>";
                //' <p> У 2019 : ' + d3.round(d.value,2) + ' динара </p>';
                //+ ' <p> 2013/2014 : ' + formatNumber(temp_rate());
              })
              .attr("class","textdiv"); //textdiv class allows us to style the text easily with CSS

            return g;
          }

            d3.json(json_file, function(root) {
                console.log(root.budget_data[3]);
                root = root.budget_data[3];
                initialize(root);
                accumulate(root);
                layout(root);
                display(root);
            });

        }
    };
})(jQuery);
