{
  "targets": [
    {
      "target_name": "dump-stacks",
      "sources": [ "src/dump-stacks.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
