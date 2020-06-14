(function ($) {
  Drupal.behaviors.opstine_budzet = {
    attach: function (context) {

      var datapath = Drupal.settings.opstine.datapath;
      var siteurl = Drupal.settings.opstine.siteurl;

      // Parameters
      var width = document.body.clientWidth*0.99;
      //var height = 2*width;

      //var height = document.body.clientHeight;
      var height = $("#main_chart").height();

      if(width < 1280){
        height = width;
      } else {
        width = height;
      }
      $("#main_chart").height(height);
      //var width = height/2;

      var padding = 0;
      var clusterPadding = 5;
      var maxRadius = 10;

      var view=1;

      var izvori_rix = {
        0: "Буџетска средства",
        1: "Сопствена средства",
        2: "Остали извори",
      };


      // Load data from variable root
      var municipality_name = root["municipality_name"];
      var year = parseInt(root["year"]);
      var data = root["budget_data"];
      var budget = [];
      data.forEach(function (item1, index1) {
        item1["razdel_programs"].forEach(function (item2, index2) {
          item2["program_activities"].forEach(function (item3, index3) {
            item3["lines"].forEach(function (item4, index4) {
              var new_bubble = {};
              new_bubble["razdel_number"]=item1["razdel_number"];
              new_bubble["razdel_name"]=item1["razdel_name"];
              new_bubble["program_id"]=item2["program_id"];
              new_bubble["program_name"]=item2["program_name"];
              new_bubble["activity_id"]=item3["activity_id"];
              new_bubble["activity_name"]=item3["activity_name"];
              new_bubble["ek_number"]=item4["ek_number"];
              new_bubble["ek_name"]=item4["ek_name"];
              new_bubble["sources"]=item4["sources"];
              new_bubble["total"]=item4["total"];
              // Previous year - for now using same values:
              new_bubble["sources_prev"]=item4["sources"];
              new_bubble["total_prev"]=item4["total"];
              budget.push(new_bubble);
            });
          });
        });
      });

      var hm=budget.length;
      var raz=[];

      var n = hm;

      var cluster_by="program_name";
      var cluster_by_name="program_name";
      //var cluster_by="ek_number"
      //var cluster_by_name="ek_name"

      // Index "cluster_by" and find maximum m
      var cluster_index = {};
      var m = 0;
      budget.forEach(function(item,index){
        var id=item[cluster_by];
        if(!(id in cluster_index)) {
          cluster_index[id]=m;
          m += 1;
        }
      });

      //var color = d3.scale.category20().domain(m);

      //
      const nazivi={
        1101: "СТАНОВАЊЕ, УРБАНИЗАМ И ПРОСТОРНО ПЛАНИРАЊЕ",
        1102: "КОМУНАЛНЕ ДЕЛАТНОСТИ",
        1501: "ЛОКАЛНИ ЕКОНОМСКИ РАЗВОЈ",
        1502: "РАЗВОЈ ТУРИЗМА",
        101: "ПОЉОПРИВРЕДА И РУРАЛНИ РАЗВОЈ",
        401: "ЗАШТИТА ЖИВОТНЕ СРЕДИНЕ",
        701: "ОРГАНИЗАЦИЈА САОБРАЋАЈА И САОБРАЋАЈНА ИНФРАСТРУКТУРА",
        2001: "ПРЕДШКОЛСКО ВАСПИТАЊЕ И ОБРАЗОВАЊЕ",
        2002: "ОСНОВНО ОБРАЗОВАЊЕ И ВАСПИТАЊЕ",
        2003: "СРЕДЊЕ ОБРАЗОВАЊЕ И ВАСПИТАЊЕ",
        901: "СОЦИЈАЛНА И ДЕЧИЈА ЗАШТИТА",
        1801: "ЗДРАВСТВЕНА ЗАШТИТА",
        1201: "РАЗВОЈ КУЛТУРЕ И ИНФОРМИСАЊА",
        1301: "РАЗВОЈ СПОРТА И ОМЛАДИНЕ",
        602: "ОПШТЕ УСЛУГЕ ЛОКАЛНЕ САМОУПРАВЕ",
        2101: "ПОЛИТИЧКИ СИСТЕМ ЛОКАЛНЕ САМОУПРАВЕ",
        501: "ЕНЕРГЕТСКА ЕФИКАСНОСТ И ОБНОВЉИВИ ИЗВОРИ ЕНЕРГИЈЕ"
      };

      const color= {
        101: "#c9b6d9",
        401: "#ed9a53",
        501: "#193ffe",
        602: "#8da2bf",
        701: "#da3c3d",
        901: "#f7bdd6",
        1101: "#ff8c26",
        1102: "#ffc185",
        1201: "#acacac",
        1301: "#a1e095",
        1501: "#41a941",
        1502: "#c2c338",
        1801: "#777777",
        2001: "#fda1a0",
        2002: "#77af9d",
        2003: "#e584c8",
        2101: "#B4CBEA",

        31:"#c9b6d9",
        32:"#ed9a53",
        71:"#193ffe",
        72:"#8da2bf",
        73:"#da3c3d",
        74:"#f7bdd6",
        75:"#ff8c26",
        76:"#ffc185",
        77:"#acacac",
        81:"#a1e095",
        82:"#41a941",
        84:"#c2c338",
        91:"#777777",
        92:"#fda1a0"
      };


      var fill = d3.scale.linear().domain([1,15,30,45,53,m]).range(["blue","green","yellow","orange","red","black"]);

      var clusters = new Array(m);

      var ak3=[], dk3={}, ddk3={};
      for(var i=0; i<n; i++) {k3=budget[i]["ek_number"]; ak3.push(k3); ddk3[k3]=budget[i]["ek_name"];}
      ak3=ak3.sort();

      var ix=0;
      for(var ii=0;ii<ak3.length;ii++) {
        if(!(ak3[ii] in dk3))
        {dk3[ak3[ii]]=ix;ix++;}
      }

      var nodes = d3.range(n).map(function(k) {
        var i = cluster_index[budget[k][cluster_by]],
          r = Math.pow(budget[k]["total"],0.33)/15.0+1,
          d = {
            cluster: i,
            radius: r,
            oradius: r,
            x: Math.cos(i / m * 2 * Math.PI) * 200 + width / 2 + Math.random(),
            y: Math.sin(i / m * 2 * Math.PI) * 200 + width / 2 + Math.random(),
            razdel_number: budget[k]["razdel_number"],
            razdel_name: budget[k]["razdel_name"],
            program_id: budget[k]["program_id"],
            program_name: budget[k]["program_name"],
            activity_id: budget[k]["activity_id"],
            activity_name: budget[k]["activity_name"],
            ek_number: budget[k]["ek_number"],
            ek_name: budget[k]["ek_name"],
            sources: budget[k]["sources"],
            sources_prev: budget[k]["sources_prev"],
            total: budget[k]["total"],
            total_prev: budget[k]["total_prev"],
            total_disp: budget[k]["total"],
            total_prev_disp: budget[k]["total_prev"]
          };
        if (!clusters[i] || (r > clusters[i].radius)) clusters[i] = d;
        return d;
      });

      var force = d3.layout.force();

      var svg = d3.select("#main_chart").append("svg")
        .attr("width", width)
        .attr("height", height);


      var node = svg.selectAll("circle")
        .data(nodes)
        .enter().append("circle")
        .style("fill", function(d) {
          return color[parseInt(d.program_id)];
        })
        .call(force.drag)
        .text("aaa")
        .on("mouseover",mousecircle)
        .on("mousemove",mousecirclemove)
        .on("mouseout",mousecircleout)
        .on("mousedown",function() {d3.event.stopPropagation();});

      node.transition()
        .duration(200)
        .delay(function(d, i) { return i * 5; })
        .attrTween("r", function(d) {
          var i = d3.interpolate(0, d.radius);
          return function(t) { return d.radius = i(t); };
        });

      function pare(s) {
        return s.toString().replace(/(\d)(?=(\d\d\d)+(?!\d))/g, "$1,");
      }

      function dispp(a,b) {
        var p=0;
        if (a !== 0) p=100.0*(b-a)/a;

        if (a===0 && b!==0) return "<font color=green>Ново</font>"
        if (p>0) return "<font color=green>+"+p.toFixed(1)+"%</font>";
        else if (p===0) return "<font color=blue>"+p.toFixed(1)+"%</font>";
        else return "<font color=red>"+p.toFixed(1)+"%</font>";
      }

      function mousecircle(d,i) {
        d3.select(this).style("stroke","#000000").style("stroke-width",1);
        $(this).popover({
          placement: "auto",
          container: "body",
          trigger: "manual",
          html : true,
          sanitize: false,
          title : function() {return "<b style='color:"+color[parseInt(d.program_id)]+";'>"+((cluster_by==="program_name") ? d.program_id : d[cluster_by] )+": "+d[cluster_by_name]+"</b>";},
          content: function() {
            return infotekst(d); }
        });
        $(this).popover("show");
      }

      function infotekst(d) {
        var s="";
        if(cluster_by !== "razdel_number") s += ""+d["razdel_number"]+": "+d.razdel_name+"<br>";
        if(cluster_by !== "program_name") s += ""+d.program_id+": "+d.program_name+"<br>";
        if(cluster_by !== "activity_id") s += ""+d.activity_id+": "+d.activity_name+"<br>";
        if(cluster_by !== "ek_number") s += ""+d.ek_number+": "+d.ek_name+"<br>";
        s += "<br>";
        var ixl=["#i01","#i02","#i03"];
        var  state=_.map(ixl,function(ii){return $(ii).is(":checked");});
        //s+="<table class='infot'><tr><th>Извор</th><th>"+(year-1).toString()+"</th><th>"+year.toString()+"</th><th>+/-</th>";
        s+="<table class='infot'><tr><th>Извор</th><th>"+year.toString()+"</th></tr>";
        for(var i in izvori_rix) {
          //	if(d.sources[i]!=0 || d.sources_prev[i]!=0)
          //s=s+"<tr><td>"+(state[i]?"<b>":"")+izvori_rix[i]+(state[i]?"</b>":"")+"<td>"+pare(d.sources_prev[i])+"<td>"+pare(d.sources[i])+"<td>"+dispp(d.sources_prev[i],d.sources[i]);
          s=s+"<tr><td>"+(state[i]?"<b>":"")+izvori_rix[i]+(state[i]?"</b>":"")+"</td><td>"+pare(d.sources[i])+"</td></tr>";
        }
        //s=s+"<tr><th><b>Укупно</b><th>"+pare(d.total_prev)+"<th>"+pare(d.total)+"<th>"+dispp(d.total_prev,d.total);
        //s=s+"<tr><th><b>Приказ</b><th>"+pare(d.total_prev_disp)+"<th>"+pare(d.total_disp)+"<th>"+dispp(d.total_prev_disp,d.total_disp);
        s=s+"<tr><th><b>Укупно</b></th><th>"+pare(d.total)+"</th></tr>";
        s=s+"<tr><th><b>Приказ</b></th><th>"+pare(d.total_disp)+"</th></tr>";
        s+="</table>";
        return s;
      }

      function mousecirclemove() {
      }

      function mousecircleout(d,i) {
        $(".popover").each(function() { $(this).remove(); });
        d3.select(this).style("stroke","#ffffff").style("stroke-width", 1);
      }

      $("#content").mousedown(function() {zadrmaj();});

      $(".btn").on("change",function() {

        ixl=["#i01","#i02","#i03"];
        state=_.map(ixl,function(ii){return $(ii).is(":checked")?1:0;});

        for(var i=0;i<nodes.length;i++) {
          nodes[i]["total_disp"]=_.reduce(_.zip(nodes[i]["sources"],state),function(a,b) {return a+b[0]*b[1];},0);
          nodes[i]["total_prev_disp"]=_.reduce(_.zip(nodes[i]["sources_prev"],state),function(a,b) {return a+b[0]*b[1];},0);

          nodes[i]["oradius"]=nodes[i]["radius"];
          nodes[i]["radius"]=Math.pow(nodes[i]["total_disp"],0.33)/15.0+1;
        }
        if (view === -1) nodes.forEach(function(o, i) {
          o.x = 450+1.2*(o.x-450);
          o.y = 350+1.2*(o.y-350);
        });
        else nodes.forEach(function(o, i) {
          o.x += (Math.random() - 0.5) * 40;
          o.y += (Math.random() - 0.5) * 40;
        });

        node.data(nodes).transition()
          .duration(1000)
          .attrTween("r", function(d) {
            var i = d3.interpolate(d.oradius, d.radius);
            return function(t) { return d.radius = i(t); };
          });
        force.resume();
      });

      function zadrmaj() {
        nodes.forEach(function(o, i) {
          o.x += (Math.random() - 0.5) * 20;
          o.y += (Math.random() - 0.5) * 20;
        });
        force.resume();
      }

      $.draw = function (id) {
        svg.selectAll(".label").remove();
        d3.select("#infoch").style('display','none');
        if(id==="l1") {
          view=1;
          force
            .nodes(nodes)
            .size([width, 0.7*width])
            .gravity(0.02)
            .charge(0)
            .on("tick", tick)
            .start();
        } else if (id==="razlike") {
          force.stop();
          d3.select("#infoch").style('display','block');
          force=d3.layout.force()
            .nodes(nodes)
            .gravity(0)
            .charge(0)
            .on("tick", function(e){
              node.each(changeSort(e.alpha))
                .attr("cx", function(d) { return d.x; })
                .attr("cy", function(d) { return d.y; });
            })
            .start();
          node.call(force.drag);
        } else if (id==="rk2") {
          force.stop();
          force=d3.layout.force()
            .nodes(nodes)
            .gravity(0)
            .charge(0)
            .on("tick", function(e){
              node.each(rk2Sort(e.alpha))
                .attr("cx", function(d) { return d.x; })
                .attr("cy", function(d) { return d.y; });
            })
            .start();
          node.call(force.drag);
        }
        else {
          view=2;
          var centers = getCenters(id, [width, height]);
          force.on("tick", tick2(centers, id));
          labels(centers);
          force.start();
        }
      };

      function changeSort(alpha) {
        return function(d){
          const hm=300,hw=250,ofx=50,mx=800/m;
          let targetY = 0;
          let targetX = 0;
          if(d.total_prev_disp > 0) {
            targetY = hm - hw * ((d.total_disp - d.total_prev_disp) / d.total_prev_disp * 4);
          }
          targetX = ofx+mx*Number(cluster_index[d[cluster_by]]);
          if (isNaN(targetY)) {targetY = 0}
          if (targetY > (hm+hw)) {targetY = hm+hw}
          if (targetY < (hm-hw)) {targetY = hm-hw}
          d.y = d.y + (targetY - d.y) * Math.sin(Math.PI * (1 - alpha*10))*0.05;
          d.x = d.x + (targetX - d.x) * Math.sin(Math.PI * (1 - alpha*10))*0.03;
        };
      }

      function rk2Sort(alpha) {
        return function(d){
          const hm=300,hw=250,ofx=150,mx=700/53,ofy=80,my=600/15;
          let targetY = 0;
          let targetX = 0;
          targetY = ofy+dk2[d.k2]*my;
          targetX = ofx+mx*Number(d.razdeo);
          if (isNaN(targetY)) {targetY = 0}
          if (targetY > (hm+hw)) {targetY = hm+hw}
          if (targetY < (hm-hw)) {targetY = hm-hw}
          d.y = d.y + (targetY - d.y) * Math.sin(Math.PI * (1 - alpha*10))*0.05;
          d.x = d.x + (targetX - d.x) * Math.sin(Math.PI * (1 - alpha*10))*0.03;
        };
      }

      function getCenters (vname, size) {
        var centers, map;
        centers = _.uniq(_.pluck(nodes, vname)).map(function (d) {
          return {name: d, value: 1};
        });
        map = d3.layout.treemap().size(size);
        map.nodes({children: centers});

        console.log(centers);

        return centers;
      }

      //hyphenation
      function getTextWidth(text, font = "500 12px sans-serif") {
        const canvas = document.createElement("canvas");
        const context = canvas.getContext("2d");
        context.font = font;
        return context.measureText(text).width;
      }
      function breakString(word, maxWidth, hyphenCharacter='-') {
        const characters = word.split("");
        const lines = [];
        let currentLine = "";
        characters.forEach((character, index) => {
          const nextLine = `${currentLine}${character}`;
          const lineWidth = getTextWidth(nextLine);
          if (lineWidth >= maxWidth) {
            const currentCharacter = index + 1;
            const isLastLine = characters.length === currentCharacter;
            const hyphenatedNextLine = `${nextLine}${hyphenCharacter}`;
            lines.push(isLastLine ? nextLine : hyphenatedNextLine);
            currentLine = "";
          } else {
            currentLine = nextLine;
          }
        });
        return { hyphenatedStrings: lines, remainingWord: currentLine };
      }
      function wrapLabel(label, maxWidth) {
        const words = (""+label).split(" ");
        const completedLines = [];
        let nextLine = "";
        words.forEach((word, index) => {
          const wordLength = getTextWidth(`${word} `);
          const nextLineLength = getTextWidth(nextLine);
          if (wordLength > maxWidth) {
            const { hyphenatedStrings, remainingWord } = breakString(word, maxWidth);
            completedLines.push(nextLine, ...hyphenatedStrings);
            nextLine = remainingWord;
          } else if (nextLineLength + wordLength >= maxWidth) {
            completedLines.push(nextLine);
            nextLine = word;
          } else {
            nextLine = [nextLine, word].filter(Boolean).join(" ");
          }
          const currentWord = index + 1;
          const isLastWord = currentWord === words.length;
          if (isLastWord) {
            completedLines.push(nextLine);
          }
        });
        return completedLines.filter(line => line !== "");
      }
      //=======================================================

      function labels (centers) {
        svg.selectAll(".label")
          .data(centers).enter().append("text")
          .attr("class", "label")
          .html(function (d) {
            var split=150;
            var txt=d.name;

            const label = wrapLabel(txt, split);
            var txt1="";
            label.forEach(function(word, index) {
              txt1 += "<tspan x='0' dy='"+(index === 0 ? 0 : "1.2em")+"'>" + word + "</tspan>";
            });
            return txt1;
          })
          .attr("transform", function (d) {
            return "translate(" + (d.x + (d.dx / 2)-20.0) + ", " + (d.y+20) + ")";
          });
      }

      function tick(e) {
        node
          .each(cluster(10 * e.alpha * e.alpha))
          .each(collide(0.5))
          .attr("cx", function(d) { return d.x; })
          .attr("cy", function(d) { return d.y; });
      }

      function tick2(centers, id) {
        var foci = {};
        for (var i = 0; i < centers.length; i++) {
          foci[centers[i].name] = centers[i];
        }
        return function (e) {
          for (var i = 0; i < nodes.length; i++) {
            var o = nodes[i];
            var f = foci[o[id]];
            o.y += ((f.y + (f.dy / 2)) - o.y) * e.alpha;
            o.x += ((f.x + (f.dx / 2)) - o.x) * e.alpha;
          }
          svg.selectAll("circle").each(collide(0.51))
            .attr("cx", function (d) { return d.x; })
            .attr("cy", function (d) { return d.y; });
        }
      }

      // Move d to be adjacent to the cluster node.
      function cluster(alpha) {
        return function(d) {
          var cluster = clusters[d.cluster];
          if (cluster === d) return;
          var x = d.x - cluster.x,
            y = d.y - cluster.y,
            l = Math.sqrt(x * x + y * y),
            r = d.radius + cluster.radius;
          if (l !== r) {
            l = (l - r) / l * alpha;
            d.x -= x *= l;
            d.y -= y *= l;
            cluster.x += x;
            cluster.y += y;
          }
        };
      }

      // Resolves collisions between d and all other circles.
      function collide(alpha) {
        var quadtree = d3.geom.quadtree(nodes);
        return function(d) {
          var r = d.radius + maxRadius + Math.max(padding, clusterPadding),
            nx1 = d.x - r,
            nx2 = d.x + r,
            ny1 = d.y - r,
            ny2 = d.y + r;
          quadtree.visit(function(quad, x1, y1, x2, y2) {
            if (quad.point && (quad.point !== d)) {
              var x = d.x - quad.point.x,
                y = d.y - quad.point.y,
                l = Math.sqrt(x * x + y * y),
                r = d.radius + quad.point.radius + (d.cluster === quad.point.cluster ? padding : clusterPadding);
              if (l < r) {
                l = (l - r) / l * alpha;
                d.x -= x *= l;
                d.y -= y *= l;
                quad.point.x += x;
                quad.point.y += y;
              }
            }
            return x1 > nx2 || x2 < nx1 || y1 > ny2 || y2 < ny1;
          });
        };
      }

      $.draw("l1");

    }
  }
})(jQuery);
