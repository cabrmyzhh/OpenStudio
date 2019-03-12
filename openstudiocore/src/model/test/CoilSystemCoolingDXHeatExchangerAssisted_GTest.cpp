/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) 2008-2019, Alliance for Sustainable Energy, LLC, and other contributors. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
*  following conditions are met:
*
*  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
*  disclaimer.
*
*  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
*  disclaimer in the documentation and/or other materials provided with the distribution.
*
*  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote products
*  derived from this software without specific prior written permission from the respective party.
*
*  (4) Other than as required in clauses (1) and (2), distributions in any form of modifications or other derivative works
*  may not use the "OpenStudio" trademark, "OS", "os", or any other confusingly similar designation without specific prior
*  written permission from Alliance for Sustainable Energy, LLC.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) AND ANY CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER(S), ANY CONTRIBUTORS, THE UNITED STATES GOVERNMENT, OR THE UNITED
*  STATES DEPARTMENT OF ENERGY, NOR ANY OF THEIR EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
*  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
*  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************************************************************/

#include <gtest/gtest.h>
#include "ModelFixture.hpp"
#include "../CoilSystemCoolingDXHeatExchangerAssisted.hpp"
#include "../CoilSystemCoolingDXHeatExchangerAssisted_Impl.hpp"

#include "../CoilCoolingDXSingleSpeed.hpp"
#include "../CoilCoolingDXSingleSpeed_Impl.hpp"
#include "../CoilCoolingDXTwoSpeed.hpp"
#include "../CoilCoolingDXVariableSpeed.hpp"
#include "../HeatExchangerAirToAirSensibleAndLatent.hpp"
#include "../HeatExchangerAirToAirSensibleAndLatent_Impl.hpp"
#include "../AirLoopHVAC.hpp"
#include "../PlantLoop.hpp"
#include "../Node.hpp"

using namespace openstudio;
using namespace openstudio::model;


TEST_F(ModelFixture,CoilSystemCoolingDXHeatExchangerAssisted)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  ASSERT_EXIT (
  {
     Model m;
     CoilSystemCoolingDXHeatExchangerAssisted valve(m);

     exit(0);
  } ,
    ::testing::ExitedWithCode(0), "" );
}

// This test ensures that only the parent CoilSystem can call addToNode, the individual CoilCoolingDXSingleSpeed and HX cannot
TEST_F(ModelFixture, CoilSystemCoolingDXHeatExchangerAssisted_addToNode) {

  Model m;
  CoilSystemCoolingDXHeatExchangerAssisted coilSystem(m);

  AirLoopHVAC a(m);
  Node n = a.supplyOutletNode();

  CoilCoolingDXSingleSpeed cc = coilSystem.coolingCoil().cast<CoilCoolingDXSingleSpeed>();
  HeatExchangerAirToAirSensibleAndLatent hx = coilSystem.heatExchanger().cast<HeatExchangerAirToAirSensibleAndLatent>();

  EXPECT_EQ(2, a.supplyComponents().size());

  EXPECT_FALSE(cc.addToNode(n));
  EXPECT_EQ(2, a.supplyComponents().size());

  EXPECT_FALSE(hx.addToNode(n));
  EXPECT_EQ(2, a.supplyComponents().size());

  EXPECT_TRUE(coilSystem.addToNode(n));
  EXPECT_EQ(3, a.supplyComponents().size());

  {
    auto containingHVACComponent = cc.containingHVACComponent();
    ASSERT_TRUE(containingHVACComponent);
    EXPECT_EQ(containingHVACComponent->handle(), coilSystem.handle());
  }

  {
    auto containingHVACComponent = hx.containingHVACComponent();
    ASSERT_TRUE(containingHVACComponent);
    EXPECT_EQ(containingHVACComponent->handle(), coilSystem.handle());
  }

}

TEST_F(ModelFixture, CoilSystemCoolingDXHeatExchangerAssisted_GettersSetters) {

  Model m;

  // Create a CoilSystem, connected to an AirLoopHVAC
  CoilSystemCoolingDXHeatExchangerAssisted coilSystem(m);
  CoilCoolingDXSingleSpeed cc = coilSystem.coolingCoil().cast<CoilCoolingDXSingleSpeed>();
  HeatExchangerAirToAirSensibleAndLatent hx = coilSystem.heatExchanger().cast<HeatExchangerAirToAirSensibleAndLatent>();

  // Check the cooling coils
  CoilCoolingDXTwoSpeed cc_bad(m);
  CoilCoolingDXVariableSpeed cc_ok(m);

  EXPECT_FALSE(coilSystem.setCoolingCoil(cc_bad));
  EXPECT_EQ(cc, coilSystem.coolingCoil());

  EXPECT_TRUE(coilSystem.setCoolingCoil(cc_ok));
  EXPECT_EQ(cc_ok, coilSystem.coolingCoil());

  HeatExchangerAirToAirSensibleAndLatent hx2(m);
  EXPECT_TRUE(coilSystem.setHeatExchanger(hx2));
  EXPECT_EQ(hx2, coilSystem.heatExchanger());

}


TEST_F(ModelFixture, CoilSystemCoolingDXHeatExchangerAssisted_clone) {

  Model m;

  // Create a CoilSystem, connected to an AirLoopHVAC
  CoilSystemCoolingDXHeatExchangerAssisted coilSystem(m);
  CoilCoolingDXSingleSpeed cc = coilSystem.coolingCoil().cast<CoilCoolingDXSingleSpeed>();
  HeatExchangerAirToAirSensibleAndLatent hx = coilSystem.heatExchanger().cast<HeatExchangerAirToAirSensibleAndLatent>();

  AirLoopHVAC a(m);
  Node n = a.supplyOutletNode();
  EXPECT_TRUE(coilSystem.addToNode(n));

  EXPECT_EQ(1u, m.getModelObjects<CoilSystemCoolingDXHeatExchangerAssisted>().size());
  EXPECT_EQ(1u, m.getModelObjects<CoilCoolingDXSingleSpeed>().size());
  EXPECT_EQ(1u, m.getModelObjects<HeatExchangerAirToAirSensibleAndLatent>().size());

  ASSERT_TRUE(coilSystem.airLoopHVAC());
  EXPECT_EQ(coilSystem.airLoopHVAC()->handle(), a.handle());

  // TODO: should these work?
  // EXPECT_TRUE(hx.airLoopHVAC());

  CoilSystemCoolingDXHeatExchangerAssisted coilSystem2 = coilSystem.clone(m).cast<CoilSystemCoolingDXHeatExchangerAssisted>();

  EXPECT_EQ(2u, m.getModelObjects<CoilSystemCoolingDXHeatExchangerAssisted>().size());
  EXPECT_EQ(2u, m.getModelObjects<CoilCoolingDXSingleSpeed>().size());
  EXPECT_EQ(2u, m.getModelObjects<HeatExchangerAirToAirSensibleAndLatent>().size());

  EXPECT_TRUE(coilSystem.airLoopHVAC());
  EXPECT_TRUE(coilSystem.inletModelObject());
  EXPECT_TRUE(coilSystem.outletModelObject());

  EXPECT_FALSE(coilSystem2.airLoopHVAC());
  EXPECT_FALSE(coilSystem2.inletModelObject());
  EXPECT_FALSE(coilSystem2.outletModelObject());

  CoilCoolingDXSingleSpeed cc2 = coilSystem2.coolingCoil().cast<CoilCoolingDXSingleSpeed>();
  HeatExchangerAirToAirSensibleAndLatent hx2 = coilSystem2.heatExchanger().cast<HeatExchangerAirToAirSensibleAndLatent>();

  EXPECT_NE(cc2.handle(), cc.handle());
  EXPECT_NE(hx2.handle(), hx.handle());

  EXPECT_FALSE(hx2.airLoopHVAC());

}
