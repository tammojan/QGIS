/***************************************************************************
                         qgsalgorithmpdalfilter.cpp
                         ---------------------
    begin                : March 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmpdalfilter.h"

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalFilterAlgorithm::name() const
{
  return QStringLiteral( "filter" );
}

QString QgsPdalFilterAlgorithm::displayName() const
{
  return QObject::tr( "Filter" );
}

QString QgsPdalFilterAlgorithm::group() const
{
  return QObject::tr( "Point cloud extraction" );
}

QString QgsPdalFilterAlgorithm::groupId() const
{
  return QStringLiteral( "pointcloudextraction" );
}

QStringList QgsPdalFilterAlgorithm::tags() const
{
  return QObject::tr( "filter,subset,extract,dimension,attribute,extent,bounds,rectangle" ).split( ',' );
}

QString QgsPdalFilterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts point from the input point cloud which match PDAL expression and/or are inside of a cropping rectangle." );
}

QgsPdalFilterAlgorithm *QgsPdalFilterAlgorithm::createInstance() const
{
  return new QgsPdalFilterAlgorithm();
}

void QgsPdalFilterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "FILTER_EXPRESSION" ), QObject::tr( "Filter expression" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "FILTER_EXTENT" ), QObject::tr( "Cropping extent" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output" ) ) );
}

QStringList QgsPdalFilterAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputName = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = { QStringLiteral( "translate" ),
                       QStringLiteral( "--input=%1" ).arg( layer->source() ),
                       QStringLiteral( "--output=%1" ).arg( outputFile )
                     };


  const QString filterExpression = parameterAsString( parameters, QStringLiteral( "FILTER_EXPRESSION" ), context ).trimmed();
  if ( !filterExpression.isEmpty() )
  {
    args << QStringLiteral( "--filter=%1" ).arg( filterExpression );
  }

  if ( parameters.value( QStringLiteral( "FILTER_EXTENT" ) ).isValid() )
  {
    if ( layer->crs().isValid() )
    {
      const QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "FILTER_EXTENT" ), context, layer->crs() );
      args << QStringLiteral( "--bounds=([%1, %2], [%3, %4])" )
           .arg( extent.xMinimum() )
           .arg( extent.xMaximum() )
           .arg( extent.yMinimum() )
           .arg( extent.yMaximum() );

    }
    else
    {
      const QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "FILTER_EXTENT" ), context );
      args << QStringLiteral( "--bounds=([%1, %2], [%3, %4])" )
           .arg( extent.xMinimum() )
           .arg( extent.xMaximum() )
           .arg( extent.yMinimum() )
           .arg( extent.yMaximum() );
    }
  }

  applyThreadsParameter( args );
  return args;
}

///@endcond
