﻿/*
 * [The "BSD license"]
 *  Copyright (c) 2013 Terence Parr
 *  Copyright (c) 2013 Dan McLaughlin
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "PredictionMode.h"

#include <assert.h>
#include "AbstractEqualityComparator.h"

namespace org {
namespace antlr {
namespace v4 {
namespace runtime {
namespace atn {

class AltAndContextConfigEqualityComparator
    : misc::AbstractEqualityComparator<ATNConfig> {
 public:
  int hashCode(ATNConfig* o);
  bool equals(ATNConfig* a, ATNConfig* b);

 private:
  AltAndContextConfigEqualityComparator() {}
};

// TODO -- Determine if we need this hash function.
int AltAndContextConfigEqualityComparator::hashCode(ATNConfig* o) {
  int hashCode = runtime::misc::MurmurHash::initialize(7);
  hashCode = runtime::misc::MurmurHash::update(hashCode, o->state->stateNumber);
  hashCode = runtime::misc::MurmurHash::update(hashCode, o->context);
  return runtime::misc::MurmurHash::finish(hashCode, 2);
}

// TODO -- Determine if we need this comparator.
bool AltAndContextConfigEqualityComparator::equals(ATNConfig* a, ATNConfig* b) {
  if (a == b) {
    return true;
  }
  if (a == nullptr || b == nullptr) {
    return false;
  }
  return a->state->stateNumber == b->state->stateNumber &&
         a->context->equals(b->context);
}

/// <summary>
/// A Map that uses just the state and the stack context as the key. </summary>
class AltAndContextMap : public std::map<ATNConfig, BitSet> {
 public:
  AltAndContextMap() {}
};

bool hasSLLConflictTerminatingPrediction(PredictionMode* mode,
                                         ATNConfigSet* configs) {
  /* Configs in rule stop states indicate reaching the end of the decision
   * rule (local context) or end of start rule (full context). If all
   * configs meet this condition, then none of the configurations is able
   * to match additional input so we terminate prediction.
   */
  if (allConfigsInRuleStopStates(configs)) {
    return true;
  }

  // pure SLL mode parsing
  if (*mode == PredictionMode::SLL) {
    // Don't bother with combining configs from different semantic
    // contexts if we can fail over to full LL; costs more time
    // since we'll often fail over anyway.
    if (configs->hasSemanticContext) {
      // dup configs, tossing out semantic predicates
      ATNConfigSet* dup = new ATNConfigSet();
      for (ATNConfig config : *configs) {
        ATNConfig* c = new ATNConfig(&config, SemanticContext::NONE);
        dup->add(c);
      }
      configs = dup;
    }
    // now we have combined contexts for configs with dissimilar preds
  }

  // pure SLL or combined SLL+LL mode parsing
  std::vector<BitSet> altsets = getConflictingAltSubsets(configs);
  bool heuristic =
      hasConflictingAltSet(altsets) && !hasStateAssociatedWithOneAlt(configs);
  return heuristic;
}

bool hasConfigInRuleStopState(ATNConfigSet* configs) {
  for (ATNConfig c : *configs) {
    if (dynamic_cast<RuleStopState*>(c.state) != NULL) {
      return true;
    }
  }

  return false;
}

bool allConfigsInRuleStopStates(ATNConfigSet* configs) {
  for (ATNConfig config : *configs) {
    if (dynamic_cast<RuleStopState*>(config.state) == NULL) {
      return false;
    }
  }

  return true;
}

int resolvesToJustOneViableAlt(const std::vector<BitSet>& altsets) {
  return getSingleViableAlt(altsets);
}

bool allSubsetsConflict(const std::vector<BitSet>& altsets) {
  return !hasNonConflictingAltSet(altsets);
}

bool hasNonConflictingAltSet(const std::vector<BitSet>& altsets) {
  for (BitSet alts : altsets) {
    if (alts.count() == 1) {
      return true;
    }
  }
  return false;
}

bool hasConflictingAltSet(const std::vector<BitSet>& altsets) {
  for (BitSet alts : altsets) {
    if (alts.count() > 1) {
      return true;
    }
  }
  return false;
}

bool allSubsetsEqual(const std::vector<BitSet>& altsets) {
  if (altsets.size() == 0) {
    // TODO -- Determine if this should return true or false when there are no
    // sets available based on the original code.
    return true;
  }
  const BitSet& first = *altsets.begin();
  for (const BitSet& alts : altsets) {
    if (alts != first) {
      return false;
    }
  }
  return true;
}

int getUniqueAlt(const std::vector<BitSet>& altsets) {
  BitSet all = getAlts(altsets);
  if (all.count() == 1) {
    // TODO -- Create a nextBit helper function.
    for (int i = 0; i < all.size(); ++i) {
      if (all[i]) {
        return i;
      }
    }
  }
  return ATN::INVALID_ALT_NUMBER;
}

BitSet getAlts(const std::vector<BitSet>& altsets) {
  BitSet all;
  for (BitSet alts : altsets) {
    all |= alts;
  }
  return all;
}

std::vector<BitSet> getConflictingAltSubsets(ATNConfigSet* configs) {
  AltAndContextMap configToAlts;
  for (const ATNConfig& c : *configs) {
    configToAlts[c].set(c.alt);
  }
  std::vector<BitSet> values;
  for (auto it : configToAlts) {
    values.push_back(it.second);
  }
  return values;
}

std::map<ATNState, BitSet> getStateToAltMap(ATNConfigSet* configs) {
  std::map<ATNState, BitSet> m;
  for (ATNConfig c : *configs) {
    m[*c.state].set(c.alt);
  }
  return m;
}

int getSingleViableAlt(const std::vector<BitSet>& altsets) {
  BitSet viableAlts;
  for (BitSet alts : altsets) {
    int minAlt = -1;
    // TODO -- Create a nextBit helper function.
    for (int i = 0; i < alts.size(); ++i) {
      if (alts[i]) {
        minAlt = i;
        break;
      }
    }
    assert(minAlt != -1);  // TODO -- Remove this after verification.
    viableAlts.set(minAlt);
    if (viableAlts.count() > 1)  // more than 1 viable alt
    {
      return ATN::INVALID_ALT_NUMBER;
    }
  }
  // TODO -- Create a nextBit helper function.
  for (int i = 0; i < viableAlts.size(); ++i) {
    if (viableAlts[i]) {
      return i;
    }
  }
  assert(false);  // TODO -- Remove this after verification.
  return -1;
}

}  // namespace atn
}  // namespace runtime
}  // namespace v4
}  // namespace antlr
}  // namespace org
