#!/bin/bash

BASE_URL="http://localhost:3001/api/engine"

echo "Testing Backend API Endpoints..."

# Health Check
echo "1. Health Check..."
curl -s http://localhost:3001/health

# Note: These will likely 500 if the C engine binary is not at core-engine/build/regm_engine
# but they prove the routes are defined.

echo -e "\n\n2. Testing /init..."
curl -s -X POST $BASE_URL/init

echo -e "\n\n3. Testing /edit..."
curl -s -X POST -H "Content-Type: application/json" -d '{"type":"Character","character":{"name":"Aris"}}' $BASE_URL/edit

echo -e "\n\n4. Testing /create..."
curl -s -X POST -H "Content-Type: application/json" -d '{"prompt":"A hero"}' $BASE_URL/create

echo -e "\n\n5. Testing /clear..."
curl -s -X POST -H "Content-Type: application/json" -d '{"imagePaths":["temp.png"]}' $BASE_URL/clear

echo -e "\n\nDone."
