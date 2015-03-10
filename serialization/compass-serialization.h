/*
 * compass-serialization.h
 *
 *  Created on: Nov 23, 2009
 *      Author: tdillig
 */

#ifndef COMPASSSERIALIZATION_H_
#define COMPASSSERIALIZATION_H_

#include <sstream>
#include <istream>
#include <ostream>
#include <fstream>
class SummaryGraph;
class AnalysisResult;
struct dotty_info;


class ProjectSettings;

class Serializable;

using namespace std;

SummaryGraph* load_summary_graph(stringstream& ss);
void write_summary_graph(stringstream & ss, SummaryGraph* sg);

AnalysisResult* load_analysis_result(stringstream & ss);
void write_analysis_results(stringstream & ss, AnalysisResult* ar);

dotty_info* load_dotty_info(stringstream& ss);
void write_dotty_info(stringstream& ss, dotty_info* di);

ProjectSettings* load_project_settings(ifstream& ss);
void write_project_settings(stringstream& ss, ProjectSettings* ps);

Serializable* load_serializable(ifstream & ss, bool sail_body);
void write_serializable(ofstream& ss, Serializable* s, bool sail_body);


#endif /* COMPASSSERIALIZATION_H_ */
