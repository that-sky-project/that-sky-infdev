const { DeclarationGroup, JsonLevelObjects } = require("sldl-jsonify");
const ps = require("process");
const fs = require("fs");

function main(argv) {
  if (argv.length < 3) {
    printUsage();
    return;
  }

  var declPath = argv[0]
    , inputPath = argv[1]
    , outputPath = argv[2];

  var declRaw = JSON.parse(fs.readFileSync(declPath, "utf-8"))
    , input = JSON.parse(fs.readFileSync(inputPath, "utf-8"));

  var decl = new DeclarationGroup(declRaw).parse()
    , objWriter = new JsonLevelObjects(decl);

  var buffer = objWriter.write(input);
  fs.writeFileSync(outputPath, buffer);
}

function printUsage() {
  console.log("Usage: node level.js <decl> <level> <output>");
}

main(ps.argv.slice(2));
