curl -X POST -H "Content-Type: application/json" -d '{"vectors": [0.8], "id": 2, "indexType": "FLAT"}'  http://localhost:7781/insert
curl -X POST -H "Content-Type: application/json" -d '{"vectors": [0.5], "k": 2, "indexType": "FLAT"}'  http://localhost:7781/search
curl -X POST -H "Content-Type: application/json" -d '{"vectors": [0.8], "k": 2, "indexType": "FLAT1"}'  http://localhost:7781/search