#pragma once
#include <fmt/format.h>
#include <boost/multi_array.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <highfive/H5File.hpp>
#include <mutex>
#include <uWS/uWS.h>
#include "rapidjson/document.h"
#include "compression.h"

typedef boost::multi_array<float, 3> Matrix3F;
typedef boost::multi_array<float, 2> Matrix2F;


struct Histogram
{
	int N;
	float firstBinCenter, binWidth;
	std::vector<int> bins;
};

struct BandStats
{
	Histogram histogram;
	float minVal;
	float maxVal;
	float mean;
	std::vector<float> percentiles;
	std::vector<float> percentileVals;
	int nanCount;
};

struct ImageInfo
{
	std::string filename;
	int numBands;
	int width;
	int height;
	std::map<int, BandStats> bandStats;
};

struct RegionReadResponse
{
	bool success;
	int compression;
	int x;
	int y;
	int w;
	int h;
	int mip;
	int band;
	int numValues;
};

struct ReadRegionRequest
{
	int x, y, w, h, band, mip, compression;
};



class Session
{
public:
	boost::uuids::uuid uuid;
protected:
	Matrix3F currentBandCache;
	Histogram currentBandHistogram;
	int currentBand;
	HighFive::File* file;
	std::vector<HighFive::DataSet> dataSets;
	ImageInfo imageInfo;
	std::mutex eventMutex;
	uWS::WebSocket<uWS::SERVER>* socket;
	std::string baseFolder;
	char* binaryPayloadCache;
	uint32_t payloadSizeCached = 0;

public:
	Session(uWS::WebSocket<uWS::SERVER>* ws, boost::uuids::uuid uuid, std::string folder);
	void onRegionRead(const rapidjson::Value& message);
	void onFileLoad(const rapidjson::Value& message);
	~Session();

protected:
	void updateHistogram();
	bool parseRegionQuery(const rapidjson::Value& message, ReadRegionRequest& regionQuery);
	bool loadFile(const std::string& filename, int defaultBand = -1);
	bool loadBand(int band);
	bool loadStats();
	std::vector<float> getZProfile(int x, int y);
	std::vector<float> readRegion(const ReadRegionRequest& req);
	void log(const std::string& logMessage);
	void profileReads();

};